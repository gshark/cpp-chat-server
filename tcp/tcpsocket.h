#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "logger.h"
#include <executor.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#include <vector>

using std::string;

class TcpSocket {
private:
    typedef std::function<void()> ClosedConnectionHandler;
    typedef std::function<void()> DataReceivedHandler;

    //TODO больше static
    static const int DEFAULT_FLAGS = EPOLLIN | EPOLLHUP;
    static const int OUT_FLAGS = EPOLLOUT | EPOLLIN | EPOLLHUP;
    static const int NONE = -1;
    //const size_t BUFFER_SIZE_ON_READ;
    //const size_t BUFFER_SIZE_ON_WRITE;

    static const size_t BUFFER_SIZE_ON_READ = 4096;
    static const size_t BUFFER_SIZE_ON_WRITE = 4096;

    //TODO host и port привести в порядок, port -> int, host -> string
    //TcpSocket(Executor *executor, int fd, string host, size_t port);
    TcpSocket(Executor *executor, int fd, sockaddr_in in_addr);
    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator = (const TcpSocket&) = delete;
    friend class TcpServerSocket;

    int fd;
    int flags;
    //string host;
    //size_t port;
    sockaddr_in in_addr;
    std::vector<char> readBuffer;
    std::vector<char> writeBuffer;

    ClosedConnectionHandler closedConnectionHandler;
    DataReceivedHandler dataReceivedHandler;
    //Logger logger;
    Executor *executor;
    bool canRead;
    bool allDataRead;
    bool* destroyedCookie = nullptr;

    void appendData(const char *s, size_t len);
    int makeSocketNonBlocking(int fd);
    bool isErrorSocket(const epoll_event&);
    void handler(const epoll_event&);
    void tryToWriteData();
public:
    enum ConnectedState { CONNECTED, ALREADY_CONNECTED, UNKNOWN_HOST};

    TcpSocket(Executor *executor);
    TcpSocket(TcpSocket&&);
    ~TcpSocket();

    ConnectedState connectToHost(const string& host, size_t port = 80);
    int getfd();
    bool write(const char *data, size_t len);
    bool write(const std::string &s);
    string readBytesFromBuffer();

    void setClosedConnectionHandler(ClosedConnectionHandler);
    void setDataReceivedHandler(DataReceivedHandler);
    void clearBuffers();
    void close();
    bool allReadCallback();
    bool isClosed();
};

#endif // TCPSOCKET_H
