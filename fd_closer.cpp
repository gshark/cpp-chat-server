#include "fd_closer.h"
#include <cassert>
#include <unistd.h>
#include <string.h>
#include <utility>
#include <signal.h>
#include <sys/signalfd.h>


fd_closer::fd_closer() : fd(-1) {}

fd_closer::fd_closer(int new_fd) : fd(new_fd) {}


fd_closer::~fd_closer() {
    ::close(fd);
    //int r = ::close(fd);
    //assert(r == 0);
}

int fd_closer::get_fd() const {
    return fd;
}

//fd_closer::fd_closer(fd_closer&& other) : fd(std::move(other.fd)) {}
fd_closer::fd_closer(fd_closer&& other) {
    fd = other.fd;
    other.fd = -1;
}


fd_closer& fd_closer::operator = (fd_closer&& other) {
    std::swap(fd, other.fd);
    return *this;
}

void fd_closer::cancel() {
    fd = -1;
}



/*chatserver*/

