#ifndef HTTPMATCHER_H
#define HTTPMATCHER_H

#include <http/httprequest.h>

class HttpMatcher
{
private:
    string method;
    string url;

    void normalize();
public:
    HttpMatcher(const string &method = "", const string &url = "");
    bool match(const HttpRequest &request);
    string getMethod();
    string getUrl();
};

#endif // HTTPMATCHER_H
