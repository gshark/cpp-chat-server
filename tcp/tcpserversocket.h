#ifndef TCPSERVERSOCKET_H
#define TCPSERVERSOCKET_H

#include <tcp/tcpsocket.h>
#include <http/httpobject.h>
#include <http/httprequest.h>
#include <memory>

using std::string;

class TcpServerSocket
{
private:
    typedef std::function<void()> NewConnectionHandler;
    typedef std::function <std::unique_ptr<TcpSocket>()> PendingConstructorHandler;

    //TODO static
    static const int MAX_EVENTS = 1024;
    //const int MAX_EVENTS;
    static const int NONE = -1;

    Executor *executor;
    int listenerfd;
    int pendingfd;
    size_t port;
    string host;
    sockaddr *adress;

    PendingConstructorHandler pendingConstructorHandler;
    NewConnectionHandler newConnectionHandler;
    //Logger logger;

    TcpServerSocket(const TcpServerSocket&) = delete;
    TcpServerSocket& operator = (const TcpServerSocket&) = delete;

    int makeSocketNonBlocking(int listenerfd);
    void handler();
    bool isErrorSocket(const epoll_event &event);
    void acceptConnection(const epoll_event &flags);
public:
    enum ConnectedState {SUCCESS, ALREADY_BINDED, ALREADY_CONNECTED};

    TcpServerSocket(Executor *executor);
    TcpServerSocket(TcpServerSocket&&);
    ~TcpServerSocket();

    ConnectedState listen(const string &host, size_t port, NewConnectionHandler newConnectionHandler);
    bool isListening();
    std::unique_ptr<TcpSocket> getPendingConnection();

    void close();
};

#endif // TCPSERVERSOCKET_H
