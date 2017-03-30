#include "chatserver.h"
#include "preparation.h"
#include <sstream>
#include <fstream>
#include <cstring>

ChatServer::ChatServer(Executor *executor) :
    httpServer(new HttpServer(executor)),
    numUsers(0) {
    httpServer->addHttpMatcher(HttpMatcher(), [this](HttpRequest request, HttpServer::Response responseHandler) {
        string filename = request.getPath();
        if (filename == "/") {
            filename = "/index.html";
        }
        if (filename != "/index.html" && filename == "/jquery.js" && filename == "/script.js") {
        /*cerr << filename << endl;
        filename = "." + filename;
        ifstream in(filename.c_str());
        if (!in) {*/
            HttpResponse responseToSend(404, "Not Found", request.getVersion(),
                           "<html>"
                           "<body>"
                           "<h1>Error 404 - Not Found</h1>"
                           "<p>The requested URL " + request.getPath() + " was not found</p>"
                           "<hr></body></html>");
            responseHandler.response(responseToSend);
            return;
        }

        /*string line;
        string result;
        while (getline(in, line)) {
            result += line + '\n';
        }*/
        string result;
        if (filename == "/index.html")
            result = preparation::INDEX_DATA;
        else if (filename == "/jquery.js")
            result = preparation::JQUERY_DATA;
        else if (filename == "/script.js")
            result = preparation::SCRIPT_DATA;
        string cookie = request.header("Cookie");
        size_t userId = getUserIdByCookie(cookie);
        if (userId == 0) {
            userId = ++numUsers;
        }
        HttpResponse responseToSend(200, "OK", request.getVersion(), result);
        if (request.isKeepAlive()) {
            responseToSend.addHeader("Connection", "Keep-Alive");
        }

        size_t pos = filename.find(".");
        string type = filename.substr(pos + 1, filename.size() - pos - 1);

        if (type == "js") {
            responseToSend.addHeader("Content-Type", "text/javascript");
        } else if (type == "html") {
            responseToSend.addHeader("Content-Type", "text/html");
        }

        responseToSend.addHeader("Set-Cookie", "user=" + std::to_string(userId) + "; expires=Fri, 31 Dec 2099 23:59:59 GMT;");
        responseToSend.addHeader("Set-Cookie", "hash=" + std::to_string(hash(userId)) + "; expires=Fri, 31 Dec 2099 23:59:59 GMT;");
        responseHandler.response(responseToSend);
    });

    httpServer->addHttpMatcher(HttpMatcher("POST", "/messages"), [=](HttpRequest request, HttpServer::Response responseHandler) {
        string cookie = request.header("Cookie");
        size_t userId = getUserIdByCookie(cookie);
        if (userId != 0) {
            if (lastReadMessage.find(userId) == lastReadMessage.end()) {
                history.push_back(Message(userId, time(0), "user" + std::to_string(userId) + " came to chat!"));
                firstReadMessage[userId] = lastReadMessage[userId] = history.size() - 1;
            }

            HttpResponse responseToSend(200, "OK", request.getVersion());
            if (request.isKeepAlive()) {
                responseToSend.addHeader("Connection", "Keep-Alive");
            }
            history.push_back(Message(userId, time(0), getMessage(request.getBody())));
            responseToSend.addHeader("Set-Cookie", "user=" + std::to_string(userId) + "; expires=Fri, 31 Dec 2099 23:59:59 GMT;");
            responseToSend.addHeader("Set-Cookie", "hash=" + std::to_string(hash(userId)) + "; expires=Fri, 31 Dec 2099 23:59:59 GMT;");
            responseHandler.response(responseToSend);
        } else {
            HttpResponse responseToSend(401, "Unauthorized", request.getVersion());
            if (request.isKeepAlive()) {
                responseToSend.addHeader("Connection", "Keep-Alive");
            }
            responseHandler.response(responseToSend);
        }
    });

    httpServer->addHttpMatcher(HttpMatcher("GET", "/messages"), [=](HttpRequest request, HttpServer::Response responseHandler) {
        string cookie = request.header("Cookie");
        size_t userId = getUserIdByCookie(cookie);

        if (userId != 0) {
            if (lastReadMessage.find(userId) == lastReadMessage.end()) {
                history.push_back(Message(0, time(0), "user" + std::to_string(userId) + " came to chat!"));
                firstReadMessage[userId] = lastReadMessage[userId] = history.size() - 1;
            }


            //QUrl url(request.getUrl().c_str());
            //QUrlQuery urlQuery(url);
            string str_url = request.getUrl();
            int leftBorder;
            if (str_url.find("all=true") != string::npos) {
                leftBorder = firstReadMessage[userId];
            } else {
                leftBorder = lastReadMessage[userId] + 1;
            }

            HttpResponse responseToSend(200, "OK", request.getVersion());
            if (request.isKeepAlive()) {
                responseToSend.addHeader("Connection", "Keep-Alive");
            }

            responseToSend.addHeader("Set-Cookie", "user=" + std::to_string(userId) + "; expires=Fri, 31 Dec 2099 23:59:59 GMT;");
            responseToSend.addHeader("Set-Cookie", "hash=" + std::to_string(hash(userId)) + "; expires=Fri, 31 Dec 2099 23:59:59 GMT;");
            responseToSend.setBody(packageToJsonHistory(leftBorder, history.size() - 1));
            lastReadMessage[userId] = history.size() - 1;
            responseHandler.response(responseToSend);
        } else {
            HttpResponse responseToSend(401, "Unauthorized", request.getVersion());
            if (request.isKeepAlive()) {
                responseToSend.addHeader("Connection", "Keep-Alive");
            }
            responseHandler.response(responseToSend);
        }
    });
}

ChatServer::~ChatServer() {
}

void ChatServer::start(int port) {
    httpServer->start(port);
}

size_t ChatServer::getUserIdByCookie(string cookie) {
    if (cookie == "") {
        return ++numUsers;
    }
    string userId = "";
    size_t index = cookie.find("user");
    if (index == string::npos) {
        return 0;
    }
    index += 5;
    while (index < cookie.size() && cookie[index] != ';') {
        userId += cookie[index++];
    }
    if (userId.size() == 0) {
        return 0;
    }

    index = cookie.find("hash");
    if (index == string::npos) {
        return 0;
    }
    index += 5;
    string hashString = "";
    while (index < cookie.size() && cookie[index] != ';') {
        hashString += cookie[index++];
    }
    if (hashString.size() == 0 || hash(atol(userId.c_str())) != (size_t)atol(hashString.c_str())) {
        return 0;
    }
    return atol(userId.c_str());
}

string ChatServer::getMessage(const string& s) {
    return s.substr(8, s.size() - 8);
}

size_t ChatServer::hash(size_t userId) {
    /*static size_t p = 239;
    string s = std::to_string(userId);
    size_t h = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        h = h * p + s[i];
    }
    return h;*/
    std::hash<size_t> hash_fn;
    return hash_fn(userId);

}

string ChatServer::packageToJsonHistory(int l, int r) {
    std::string result = "{\"messages\": [";
    for (; l <= r; ++l) {
        if (l == r) {
            result += history[l].toJson();
        } else {
            result += history[l].toJson() + ", ";
        }
    }
    return result + "]}";
}

string ChatServer::Message::toJson() {
    return "{\"from\": " + std::to_string(from) + ", \"timestamp\": " + std::to_string(time) + ", \"text\": \"" + text + "\"}";
}
