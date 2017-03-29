#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "logger.h"
#include "fd_closer.h"

#include <sys/epoll.h>

#include <functional>
#include <map>
#include <vector>

using namespace std;

class Executor {
private:
    typedef std::function <void(epoll_event)> EventHandler;
    static const int MAX_EVENTS = 1024;
    //const int MAX_EVENTS;

    //Logger logger;
    fd_closer globalfd;
    fd_closer sigfd;
    //int globalfd;
    //int sigfd;
    std::vector<epoll_event> events;
    std::map<int, EventHandler> handlers;
public:
    Executor();
    ~Executor();
    Executor(const Executor&) = delete;
    Executor& operator = (const Executor&) = delete;


    int setHandler(int fd, EventHandler handler, uint32_t flags);
    void removeHandler(int fd);
    void changeFlags(int fd, uint32_t flags);
    int execute();
    void stop();
};

#endif // EXECUTOR_H
