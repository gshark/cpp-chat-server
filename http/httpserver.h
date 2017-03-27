#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <tcp/tcpserversocket.h>
#include <http/httprequest.h>
#include <http/httpresponse.h>
#include <http/httpmatcher.h>
#include <http/httpobject.h>
#include <executor.h>
#include <logger.h>

#include <set>

class HttpServer {
public:
    struct Connection {
        Connection(HttpServer* server, std::unique_ptr<TcpSocket> socket);

        HttpServer* server;
        std::unique_ptr<TcpSocket> socket;
        HttpRequest request;
        HttpResponse response;
    };

    struct Response {
        bool response(const HttpResponse &response);
        Response(TcpSocket *socket);
        TcpSocket *socket;
    };

    typedef std::function <void(HttpRequest, Response)> RequestHandler;
    //enum ServerStatus {SUCCESS, ALREADY_BINDED, ALREADY_STARTED};

    HttpServer(Executor *executor);
    ~HttpServer();

    void start(int port);
    void addHttpMatcher(const HttpMatcher &matcher, const RequestHandler &handler);
    void finish(Connection* conn);
private:
    //Logger logger;
    Executor *executor;
    std::vector<std::pair<HttpMatcher, RequestHandler>> matchers;
    TcpServerSocket listener;

    std::map<TcpSocket*, std::unique_ptr<Connection>> connections;

    void readRequest(std::unique_ptr<TcpSocket> socket);
};

#endif // HTTPSERVER_H
