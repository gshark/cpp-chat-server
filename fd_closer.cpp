#include "fd_closer.h"
#include <cassert>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/signalfd.h>


fd_closer::fd_closer() {}

fd_closer::fd_closer(int new_fd) : fd(new_fd) {}


fd_closer::~fd_closer() {
    //::close(fd);
    int r = ::close(fd);
    assert(r == 0);
}


/*chatserver*/

