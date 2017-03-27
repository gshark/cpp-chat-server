#include "executor.h"
#include <logger.h>

#include <cassert>
#include <iostream>
#include <iostream>

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/signalfd.h>

#include "fd_closer.h"

Executor::Executor() :
    //MAX_EVENTS(1024),
    globalfd(epoll_create1(0)),
    events(MAX_EVENTS) {
    //globalfd = epoll_create1(0);
    //globalfd = fd_closer(epoll_create1(0));
    //globalfd = fd_closer(epoll_create1(0));
    //globalfd.fd = epoll_create1(0);
    //TODO проверить ошибку
    if (globalfd.get_fd() == -1) {
        Logger::error("An error occurred in Executor::Executor() in epoll_create1(): " + std::string(strerror(errno)));
        throw std::runtime_error("Executor::Executor(), epoll_create1() failed");
    }
    sigset_t mask;
    int r = sigemptyset(&mask);
    if (r == -1) {
        Logger::error("An error occurred in Executor::Executor() in sigemptyset(): " + std::string(strerror(errno)));
        throw std::runtime_error("Executor::Executor(), sigemptyset() failed");
    }
    //r = sigaddset(&mask, SIGINT);
    r = sigaddset(&mask, SIGINT);
    if (r == -1) {
        Logger::error("An error occurred in Executor::Executor() in sigaddset(SIGINT): " + std::string(strerror(errno)));
        throw std::runtime_error("Executor::Executor(), sigaddset(SIGINT) failed");
    }
    r = sigaddset(&mask, SIGTERM);
    if (r == -1) {
        Logger::error("An error occurred in Executor::Executor() in sigaddset(SIGTERM): " + std::string(strerror(errno)));
        throw std::runtime_error("Executor::Executor(), sigaddset(SIGTERM) failed");
    }

    if (sigprocmask(SIG_BLOCK, &mask, 0) == -1) {
        Logger::error("An error occurred in Executor::Executor() while trying to set new block signals (sigprocmask)" + std::string(strerror(errno)));
        throw std::runtime_error("Executor::Executor(), sigprocmask(SIG_BLOCK) failed");
    }
    //sigfd = signalfd(-1, &mask, 0);
    //if (sigfd == -1) {
    //sigfd = std::move(fd_closer(signalfd(-1, &mask, 0)));
    sigfd = fd_closer(signalfd(-1, &mask, 0));
    //sigfd.fd = signalfd(-1, &mask, 0);
    if (sigfd.get_fd() == -1) {
        Logger::error("An error occurred in Executor::Executor() in signalfd()" + std::string(strerror(errno)));
        throw std::runtime_error("Executor::Executor(), signalfd() failed");
    }
    epoll_event event = {};
    //memset(&event, 0, sizeof event);
    event.data.fd = sigfd.get_fd();

    //event.data.fd = sigfd;
    event.events = EPOLLIN;
    //r = epoll_ctl(globalfd, EPOLL_CTL_ADD, sigfd, &event);
    r = epoll_ctl(globalfd.get_fd(), EPOLL_CTL_ADD, sigfd.get_fd(), &event);

    if (r == -1) {
        Logger::error("An error occurred in Executor::Executor() in epoll_ctl() " + std::string(strerror(errno)));
        throw std::runtime_error("Executor::Executor(), epoll_ctl() failed");
    }
}


//TODO убить memsetы

int Executor::setHandler(int fd, EventHandler handler, uint32_t flags) {
    epoll_event event;
    //memset(&event, 0, sizeof event);
    event.data.fd = fd;
    event.events = flags;
    //TODO результаты find в iterator
    std::map<int, EventHandler>::iterator found_it = handlers.find(fd);
    if (found_it != handlers.end()) {
        int exitCode = epoll_ctl(globalfd.get_fd(), EPOLL_CTL_MOD, fd, &event);
        if (exitCode == -1) {
            Logger::error("An error occurred in Executor::setHandler() in epoll_ctl(EPOLL_CTL_MOD)" + std::string(strerror(errno)));
            throw std::runtime_error("Executor::setHandler(), epoll_ctl(EPOLL_CTL_MOD) failed");
        }
        found_it->second = handler;
        //handlers[fd] = handler;
        return exitCode;
    }
    int exitCode = epoll_ctl(globalfd.get_fd(), EPOLL_CTL_ADD, fd, &event);
    if (exitCode == 0) {
        handlers.insert(found_it, std::make_pair(fd, handler));
        //found_it->second = handler;
        //handlers[fd] = handler;
    } else {
        Logger::error("An error occurred in Executor::setHandler() in epoll_ctl(EPOLL_CTL_ADD)" + std::string(strerror(errno)));
        throw std::runtime_error("Executor::setHandler(), epoll_ctl(EPOLL_CTL_ADD) failed");
    }
    return exitCode;
}

void Executor::changeFlags(int fd, uint32_t flags) {
    epoll_event event;
    //memset(&event, 0, sizeof event);
    event.events = flags;
    event.data.fd = fd;
    int r = epoll_ctl(globalfd.get_fd(), EPOLL_CTL_MOD, fd, &event);
    if (r == -1) {
        Logger::error("An error occurred in Executor::changeFlags() in epoll_ctl(EPOLL_CTL_MOD)" + std::string(strerror(errno)));
        throw std::runtime_error("Executor::changeFlags(), epoll_ctl(EPOLL_CTL_MOD) failed");
    }
}

void Executor::removeHandler(int fd) {
    std::map<int, EventHandler>::iterator found_it = handlers.find(fd);
    //assert(found_it != handlers.end());
    if (handlers.find(fd) == handlers.end()) {
        //TODO ?? exception или assert
        return;
    }
    handlers.erase(found_it);
    int r = epoll_ctl(globalfd.get_fd(), EPOLL_CTL_DEL, fd, 0);
    if (r == -1) {
        Logger::error("An error occurred in Executor::removeHandler() in epoll_ctl(EPOLL_CTL_DEL)" + std::string(strerror(errno)));
        throw std::runtime_error("Executor::removeHandler(), epoll_ctl(EPOLL_CTL_DEL) failed");
    }
}

int Executor::execute() {
    while (true) {
        int eventsCount = epoll_wait(globalfd.get_fd(), events.data(), events.size(), -1);
        if (eventsCount == -1) {
            Logger::error("An error occurred in Executor::execute() in epoll_wait()" + std::string(strerror(errno)));
            throw std::runtime_error("Executor::execute(), epoll_wait() failed");
        }
        bool stop = false;
        for (int i = 0; i < eventsCount; ++i) {
            if (events[i].data.fd == sigfd.get_fd()) {
                stop = true;
                break;
            }
        }
        if (stop) {
            break;
        }
        for (int i = 0; i < eventsCount; ++i)
            handlers[events[i].data.fd](events[i]);
    }
    return 0;
}

Executor::~Executor() {
    /*int r = ::close(globalfd);
    assert(r == 0);
    r = ::close(sigfd);
    assert(r == 0);*/
}
