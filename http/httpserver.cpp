#include "httpserver.h"
#include <http/httputils.h>

#include <iostream>
#include <cassert>

HttpServer::HttpServer(Executor *executor) :
    executor(executor),
    listener(executor) {}

void HttpServer::start(int port) {
    TcpServerSocket::ConnectedState state = listener.listen(port, [this]() {
        readRequest(listener.getPendingConnection());
    });

    if (state == TcpServerSocket::ALREADY_BINDED) {
        throw std::runtime_error("Already bound");
    }
    if (state == TcpServerSocket::ALREADY_CONNECTED) {
        throw std::runtime_error("Already started");
    }
}

HttpServer::~HttpServer() {
}

void HttpServer::addHttpMatcher(const HttpMatcher &matcher, const RequestHandler &handler) {
    matchers.push_back(std::make_pair(matcher, handler));
}

void HttpServer::finish(HttpServer::Connection *conn) {
    bool found = false;
    for (size_t i = 0; i != matchers.size(); ++i) {
        if (matchers[i].first.match(conn->request)) {
            matchers[i].second(conn->request, Response(conn->socket.get()));
            found = true;
        }
    }

    if (!found) {
        for (size_t i = 0; i != matchers.size(); ++i) {
            if (matchers[i].first.getMethod() == "" && matchers[i].first.getUrl() == "") {
                matchers[i].second(conn->request, Response(conn->socket.get()));
            }
        }
    }
    if (conn->socket->allReadCallback() || !conn->request.isKeepAlive()) {
        connections.erase(conn->socket.get());
    }
}

HttpServer::Response::Response(TcpSocket *socket) :
    socket(socket) {}

bool HttpServer::Response::response(const HttpResponse &response) {
    if (socket) {
        socket->write(response.toString());
        socket = 0;
        return true;
    }
    return false;
}

void HttpServer::readRequest(std::unique_ptr<TcpSocket> socket) {
    if (socket == nullptr) {
        return;
    }

    TcpSocket *sock_ptr = socket.get();
    std::unique_ptr<Connection> conn(new Connection(this, std::move(socket)));
    connections.insert(std::make_pair(sock_ptr, std::move(conn)));
}

HttpServer::Connection::Connection(HttpServer* server_arg, std::unique_ptr<TcpSocket> socket_arg)
    : server(server_arg)
    , socket(std::move(socket_arg))
    , request(HttpObject::DYNAMIC)
    , response(HttpObject::DYNAMIC) {
    socket->setClosedConnectionHandler([this]() mutable {
        server->connections.erase(socket.get());
    });

    socket->setDataReceivedHandler([this]() mutable {
        try {
            request.append(socket->readBytesFromBuffer());
            if (request.hasBody() && (int)request.getBody().size() == request.contentLength()) {
                request.commit();
                server->finish(this);
            }
        } catch (...) {
            server->connections.erase(socket.get());
        }
    });
}
