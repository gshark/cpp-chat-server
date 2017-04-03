#include "tcpserversocket.h"
#include <iostream>
#include <cassert>
#include <cstring>

#define testBit(mask, bit) ((mask) & (bit))

TcpServerSocket::TcpServerSocket(Executor *executor) :
    executor(executor),
    listenerfd(fd_closer::NONE) {
}

TcpServerSocket::TcpServerSocket(TcpServerSocket &&other) :
    in_addr(other.in_addr) {
    swap(listenerfd, other.listenerfd);
    newConnectionHandler.swap(other.newConnectionHandler);
}

void TcpServerSocket::close() {
    if (listenerfd.get_fd() == fd_closer::NONE) {
        return;
    }
    Logger::info("Closing server connection on " + std::to_string(listenerfd.get_fd()));
    executor->removeHandler(listenerfd.get_fd());
    int r = ::shutdown(listenerfd.get_fd(), SHUT_RDWR);
    if (r != 0 && errno != ENOTCONN) {
        Logger::error(std::string("Shutdown error: ") + strerror(errno));
        throw std::runtime_error("TcpSocket::close(), shutdown() failed");
    }
}

TcpServerSocket::~TcpServerSocket() {
    try {
        close();
    } catch (std::exception const &e) {
        Logger::error("TcpServerSocket::~TcpServerSocket() failed: " + std::string(e.what()));
    }

    newConnectionHandler = NewConnectionHandler();
}

TcpServerSocket::ConnectedState TcpServerSocket::listen(size_t port, NewConnectionHandler newConnectionHandler) {
    if (listenerfd.get_fd() != fd_closer::NONE) {
        return ALREADY_CONNECTED;
    }
    listenerfd = fd_closer(socket(AF_INET, SOCK_STREAM, 0));
    if (listenerfd.get_fd() == fd_closer::NONE) {
        Logger::error("An error occurred in TcpServerSocket::listen(), in socket(): " + std::string(strerror(errno)));
        throw std::runtime_error("TcpServerSocket::listen(), socket() failed");
    }
    Logger::info("Opening server connection on " + std::to_string(listenerfd.get_fd()));

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    int status = 0;
    if (setsockopt(listenerfd.get_fd(), SOL_SOCKET, SO_REUSEADDR, &status, sizeof(int)) == -1) {
        Logger::error("An error occurred in TcpServerSocket::listen(), in setsockopt(): " + std::string(strerror(errno)));
        throw std::runtime_error("TcpServerSocket::listen(), setsockopt() failed");
    }
    if (bind(listenerfd.get_fd(), (sockaddr*)&serverAddress, sizeof serverAddress) != 0) {
        /*int r = ::close(listenerfd);
        assert(r == 0);
        listenerfd = NONE;*/
        return ALREADY_BINDED;
    }

    int s;
    s = makeSocketNonBlocking(listenerfd.get_fd());
    if (s == fd_closer::NONE) {
        throw std::runtime_error("TcpServerSocket::listen(), makeSocketNonBlocking() failed");
    }
    s = ::listen(listenerfd.get_fd(), SOMAXCONN);
    if (s == -1) {
        Logger::error("An error occurred in TcpServerSocket::listen(), in ::listen(): " + std::string(strerror(errno)));
        throw std::runtime_error("TcpServerSocket::listen(), ::listen() failed");
    }

    executor->setHandler(listenerfd.get_fd() , [this](const epoll_event &event) {
        acceptConnection(event);
    }, EPOLLIN);

    this->in_addr = serverAddress;
    this->newConnectionHandler = newConnectionHandler;
    return SUCCESS;
}

bool TcpServerSocket::isListening() {
    return listenerfd.get_fd() != fd_closer::NONE;
}

int TcpServerSocket::makeSocketNonBlocking(int listenerfd) {
    int flags = fcntl(listenerfd, F_GETFL, 0);
    if (flags == -1) {
        Logger::error("An error occurred in TcpServerSocket::makeSocketNonBlocking in getting access to file " + std::string(gai_strerror(flags)));
        return fd_closer::NONE;
    }

    if (fcntl(listenerfd, F_SETFL, flags | O_NONBLOCK) != 0) {
        Logger::error("An error occurred in TcpServerSocket::makeSocketNonBlocking in setting flags" + std::string(strerror(errno)));
        return fd_closer::NONE;
    }
    return 0;
}

void TcpServerSocket::acceptConnection(const epoll_event &event) {
    assert(!isErrorSocket(event));
    if (newConnectionHandler) {
        newConnectionHandler();
    }
}

std::unique_ptr<TcpSocket> TcpServerSocket::getPendingConnection() {
    sockaddr_in in_addr;
    socklen_t in_len = sizeof(in_addr);
    int incomingfd = accept(listenerfd.get_fd(), (sockaddr*)&in_addr, &in_len);
    if (incomingfd == fd_closer::NONE) {
        Logger::error("An error occurred in TcpServerSocket::acceptConnection() in accept(): " + std::string(strerror(errno)));
    }
    fd_closer incomingfd_closer = fd_closer(incomingfd);
    if (incomingfd_closer.get_fd() == fd_closer::NONE) {
        return std::unique_ptr<TcpSocket>(nullptr);
    }
    int r = makeSocketNonBlocking(incomingfd_closer.get_fd());
    if (r == fd_closer::NONE) {
        throw std::runtime_error("TcpServerSocket::acceptConnection(), makeSocketNonBlocking() failed in lambda function pendingConstructorHandler()");
    }
    std::unique_ptr<TcpSocket> socket(new TcpSocket(executor, std::move(incomingfd_closer), in_addr));
    return std::move(socket);
}

bool TcpServerSocket::isErrorSocket(const epoll_event& event) {
    return testBit(event.events, EPOLLERR) ||
           testBit(event.events, EPOLLHUP) ||
          !testBit(event.events, EPOLLIN);
}
