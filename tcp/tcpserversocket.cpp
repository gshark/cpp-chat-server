#include "tcpserversocket.h"

#include <cassert>
#include <cstring>

#define testBit(mask, bit) ((mask) & (bit))

TcpServerSocket::TcpServerSocket(Executor *executor) :
    //MAX_EVENTS(1024),
    executor(executor),
    listenerfd(NONE) {
}

TcpServerSocket::TcpServerSocket(TcpServerSocket &&other) :
    //MAX_EVENTS(other.MAX_EVENTS),
    listenerfd(other.listenerfd),
    pendingfd(other.pendingfd),
    in_addr(other.in_addr) {
    //port(other.port) {
    //host.swap(other.host);
    pendingConstructorHandler.swap(other.pendingConstructorHandler);
    newConnectionHandler.swap(other.newConnectionHandler);
}

void TcpServerSocket::close() {
    if (listenerfd == NONE) {
        return;
    }
    Logger::info("Closing server connection on " + std::to_string(listenerfd));
    executor->removeHandler(listenerfd);
    int r = ::shutdown(listenerfd, SHUT_RDWR);
    if (r != 0 && errno != ENOTCONN) {
        Logger::error(std::string("Shutdown error: ") + strerror(errno));
        r = ::close(listenerfd);
        throw std::runtime_error("TcpSocket::close(), shutdown() failed");
    }
    //TODO если exception надо все равно закрыть
    r = ::close(listenerfd);
    assert(r == 0);
    listenerfd = NONE;
    //port = 0;
    //host = "";
}

TcpServerSocket::~TcpServerSocket() {
    try {
        close();
    } catch (std::exception const &e) {
        Logger::error("TcpServerSocket::~TcpServerSocket() failed: " + std::string(e.what()));
    }

    pendingConstructorHandler = PendingConstructorHandler();
    newConnectionHandler = NewConnectionHandler();
}

TcpServerSocket::ConnectedState TcpServerSocket::listen(size_t port, NewConnectionHandler newConnectionHandler) {
//TcpServerSocket::ConnectedState TcpServerSocket::listen(const string &host, size_t port, NewConnectionHandler newConnectionHandler) {
    if (listenerfd != NONE) {
        return ALREADY_CONNECTED;
    }
    listenerfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenerfd == -1) {
        Logger::error("An error occurred in TcpServerSocket::listen(), in socket(): " + std::string(strerror(errno)));
        throw std::runtime_error("TcpServerSocket::listen(), socket() failed");
    }
    Logger::info("Opening server connection on " + std::to_string(listenerfd));

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    int status = 0;
    if (setsockopt(listenerfd, SOL_SOCKET, SO_REUSEADDR, &status, sizeof(int)) == -1) {
        Logger::error("An error occurred in TcpServerSocket::listen(), in setsockopt(): " + std::string(strerror(errno)));
        throw std::runtime_error("TcpServerSocket::listen(), setsockopt() failed");
    }
    if (bind(listenerfd, (sockaddr*)&serverAddress, sizeof serverAddress) != 0) {
        int r = ::close(listenerfd);
        assert(r == 0);
        listenerfd = NONE;
        return ALREADY_BINDED;
    }

    int s;
    s = makeSocketNonBlocking(listenerfd);
    if (s == NONE) {
        throw std::runtime_error("TcpServerSocket::listen(), makeSocketNonBlocking() failed");
    }
    s = ::listen(listenerfd, SOMAXCONN);
    if (s == -1) {
        Logger::error("An error occurred in TcpServerSocket::listen(), in ::listen(): " + std::string(strerror(errno)));
        throw std::runtime_error("TcpServerSocket::listen(), ::listen() failed");
    }

    executor->setHandler(listenerfd , [this](const epoll_event &event) {
        acceptConnection(event);
    }, EPOLLIN);

    //this->port = port;
    //this->host = host;
    this->in_addr = serverAddress;
    this->newConnectionHandler = newConnectionHandler;
    return SUCCESS;
}

bool TcpServerSocket::isListening() {
    return listenerfd != NONE;
}

int TcpServerSocket::makeSocketNonBlocking(int listenerfd) {
    int flags = fcntl(listenerfd, F_GETFL, 0);
    if (flags == -1) {
        Logger::error("An error occurred in TcpServerSocket::makeSocketNonBlocking in getting access to file " + std::string(gai_strerror(flags)));
        return NONE;
    }

    if (fcntl(listenerfd, F_SETFL, flags | O_NONBLOCK) != 0) {
        Logger::error("An error occurred in TcpServerSocket::makeSocketNonBlocking in setting flags" + std::string(strerror(errno)));
        return NONE;
    }
    return 0;
}

void TcpServerSocket::acceptConnection(const epoll_event &event) {
    if (isErrorSocket(event)) {
        return;
    }

    sockaddr_in in_addr;
    socklen_t in_len = sizeof(in_addr);
    int incomingfd = accept(listenerfd, (sockaddr*)&in_addr, &in_len);
    if (incomingfd == -1) {
        Logger::error("An error occurred in TcpServerSocket::acceptConnection() in accept(): " + std::string(strerror(errno)));
    }
    pendingConstructorHandler = [=]() {
        if (incomingfd == -1) {
            return std::unique_ptr<TcpSocket>(nullptr);
            //throw std::runtime_error("TcpServerSocket::acceptConnection, lambda function pendingConstructorHandler: accept() failed");
        }
        int r = makeSocketNonBlocking(incomingfd);
        if (r == NONE) {
            throw std::runtime_error("TcpServerSocket::acceptConnection(), makeSocketNonBlocking() failed in lambda function pendingConstructorHandler()");
        }
        char hostbuf[NI_MAXHOST], portbuf[NI_MAXSERV];
        memset(hostbuf, 0, sizeof hostbuf);
        memset(portbuf, 0, sizeof portbuf);
        r = getnameinfo((sockaddr*)&in_addr, in_len, hostbuf, sizeof hostbuf, portbuf, sizeof portbuf, NI_NUMERICHOST | NI_NUMERICSERV);
        if (r != 0) {
            Logger::error("An error occurred in TcpServerSocket::acceptConnection() in getnameinfo() in lambda function pendingConstructorHandler(): " + std::string(gai_strerror(r)));
            throw std::runtime_error("TcpServerSocket::acceptConnection(), getnameinfo() failed");
        }
        //size_t port = ntohs(in_addr.sin_port);
        //std::unique_ptr<TcpSocket> socket(new TcpSocket(executor, incomingfd, string(hostbuf), port));
        std::unique_ptr<TcpSocket> socket(new TcpSocket(executor, incomingfd, in_addr));
        return std::move(socket);
    };
    if (newConnectionHandler) {
        newConnectionHandler();
    }
    if (pendingConstructorHandler) {
        Logger::info("Closing connection on descriptor " + std::to_string(incomingfd));
        int r = ::close(incomingfd);
        assert(r == 0);
    }
    pendingConstructorHandler = PendingConstructorHandler();
}

std::unique_ptr<TcpSocket> TcpServerSocket::getPendingConnection() {
    if (!pendingConstructorHandler) {
        return 0;
    }
    auto socket = pendingConstructorHandler();
    pendingConstructorHandler = PendingConstructorHandler();
    return std::move(socket);
}

bool TcpServerSocket::isErrorSocket(const epoll_event& event) {
    return testBit(event.events, EPOLLERR) ||
           testBit(event.events, EPOLLHUP) ||
          !testBit(event.events, EPOLLIN);
}
