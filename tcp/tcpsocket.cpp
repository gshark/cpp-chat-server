#include "tcpsocket.h"
#include <iostream>
#include <sys/types.h>
#include <sys/epoll.h>
#include <signal.h>

#include <cassert>
#include <cstring>

#define testBit(mask, bit) ((mask) & (bit))

TcpSocket::TcpSocket(Executor *executor, fd_closer fdi, sockaddr_in in_addr) :
    fd(std::move(fdi)),
    flags(DEFAULT_FLAGS),
    in_addr(in_addr),
    executor(executor),
    canRead(true),
    allDataRead(false) {
    executor->setHandler(fd.get_fd(), [this](const epoll_event &event) {
        handler(event);
    }, TcpSocket::DEFAULT_FLAGS);
    Logger::info("Opened connection on descriptor " + std::to_string(fd.get_fd()));
}
void TcpSocket::close() {
    if (fd.get_fd() == fd_closer::NONE) {
        return;
    }
    Logger::info("Closing connection on descriptor " + std::to_string(fd.get_fd()));
    clearBuffers();
    executor->removeHandler(fd.get_fd());
    int r = ::shutdown(fd.get_fd(), SHUT_RDWR);
    if (r != 0 && errno != ENOTCONN) {
        Logger::error(std::string("Shutdown error: ") + strerror(errno));
        throw std::runtime_error("TcpSocket::close(), shutdown() failed");
    }
    in_addr = {};
    canRead = false;
    allDataRead = true;
}

TcpSocket::~TcpSocket() {
    if (destroyedCookie) {
        *destroyedCookie = true;
    }
    try {
        close();
    } catch (std::exception const &e) {
        Logger::error("TcpSocket::~TcpSocket() failed: " + std::string(e.what()));
    }
}

void TcpSocket::handler(const epoll_event &event) {
    bool destroyed = false;
    assert(destroyedCookie == nullptr);
    destroyedCookie = &destroyed;

    if (testBit(event.events, EPOLLHUP) && closedConnectionHandler) {
        closedConnectionHandler();
        if (destroyed)
            return;
    }

    if (testBit(event.events, EPOLLOUT)) {
        tryToWriteData();
        if (destroyed)
            return;
    }

    canRead = false;
    if (testBit(event.events, EPOLLIN) && !allDataRead) {
        bool reachedEndOfFile = false;
        bool noBytesRead = true;
        char buffer[BUFFER_SIZE_ON_READ];
        while (true) {
            ssize_t readBytes = ::read(fd.get_fd(), buffer, BUFFER_SIZE_ON_READ);
            if (readBytes == -1) {
                if (errno != EAGAIN) {
                    reachedEndOfFile = true;
                }
                break;
            } else if (readBytes == 0) {
                reachedEndOfFile = true;
                break;
            } else {
                noBytesRead = false;
                readBuffer.insert(readBuffer.end(), buffer, buffer + readBytes);
            }
        }
        if (!noBytesRead && dataReceivedHandler) {
            dataReceivedHandler();
            if (destroyed)
                return;
        }
        canRead = !writeBuffer.empty() || !reachedEndOfFile;
        if (reachedEndOfFile) {
            allDataRead = true;
        }
    }

    assert(destroyedCookie == &destroyed);
    destroyedCookie = nullptr;
}

bool TcpSocket::isErrorSocket(const epoll_event &event) {
    return testBit(event.events, EPOLLERR) ||
           testBit(event.events, EPOLLHUP) ||
          !testBit(event.events, EPOLLIN) ||
          !testBit(event.events, EPOLLOUT);
}


bool TcpSocket::write(const char *data, size_t len) {
    if (fd.get_fd() == fd_closer::NONE) {
        return false;
    }
    appendData(data, len);
    tryToWriteData();
    return true;
}

bool TcpSocket::write(const string &s) {
    return TcpSocket::write(s.data(), s.size());
}

string TcpSocket::readBytesFromBuffer() {
    string bytes = "";
    for (size_t i = 0; i != readBuffer.size(); ++i) {
        bytes += readBuffer[i];
    }
    readBuffer.clear();
    return bytes;
}

void TcpSocket::setClosedConnectionHandler(ClosedConnectionHandler handler) {
    closedConnectionHandler = handler;
}

void TcpSocket::setDataReceivedHandler(DataReceivedHandler handler) {
    dataReceivedHandler = handler;
    if (handler && !readBuffer.empty()) {
        handler();
    }
}

void TcpSocket::clearBuffers() {
    readBuffer.clear();
    writeBuffer.clear();
}

void TcpSocket::appendData(const char *s, size_t len) {
    for (size_t i = 0; i != len; ++i) {
        writeBuffer.push_back(s[i]);
    }
}

void TcpSocket::tryToWriteData() {
    char buffer[BUFFER_SIZE_ON_WRITE];
    size_t bufferSize = 0;
    while (!writeBuffer.empty()) {
        auto it = writeBuffer.begin();
        for (; bufferSize < BUFFER_SIZE_ON_WRITE && it != writeBuffer.end(); ++it) {
            buffer[bufferSize++] = *it;
        }
        ssize_t writtenBytes = ::write(fd.get_fd(), buffer, bufferSize);
        if (writtenBytes == -1) {
            break;
        }
        bufferSize -= writtenBytes;
        writeBuffer.erase(writeBuffer.begin(), writeBuffer.begin() + writtenBytes);
    }
    if (writeBuffer.empty() && flags != DEFAULT_FLAGS) {
        executor->changeFlags(fd.get_fd(), (flags = DEFAULT_FLAGS));
    } else if (flags != OUT_FLAGS) {
        executor->changeFlags(fd.get_fd(), (flags = OUT_FLAGS));
    }
}

bool TcpSocket::allReadCallback() {
    return !canRead;
}

bool TcpSocket::isClosed() {
    return fd.get_fd() == fd_closer::NONE;
}
