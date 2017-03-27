#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <executor.h>
#include <http/httpserver.h>
#include <memory>
#include <iostream>

//#include <QUrlQuery>

using std::string;

class ChatServer {
private:
    class Message {
    public:
        Message(int from, int time, const string &text):
            from(from),
            time(time),
            text(text) {
        }

        int from;
        int time;
        string text;
        string toJson();
        friend class ChatServer;
    };

    //Logger logger;
    std::unique_ptr<HttpServer> httpServer;
    string getStringByFile(const char *name);
    size_t numUsers;
    size_t hash(size_t userId);
    std::map<size_t, int> firstReadMessage, lastReadMessage;

    std::vector<Message> history;
    size_t getUserIdByCookie(string cookie);
    string getMessage(const string &str);
    string packageToJsonHistory(int l, int r);
public:
    ChatServer(Executor *executor);
    ~ChatServer();

    int start(int port);
};

#endif // CHATSERVER_H
