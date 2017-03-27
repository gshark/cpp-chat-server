#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <http/httpobject.h>

class HttpRequest : public HttpObject {
private:
    enum Method {HEAD, GET, PUT, POST, DELETE};

    string getMethod(Method method);
    void splitUrl(string url, string &host, string &path);

    string method;
    string path;
protected:
    void parseFirstLine(const string &line);
public:
    HttpRequest(CreationMode mode = STATIC);
    HttpRequest(Method method, const string &url,
                const string &body = "", const string &version = "1.0");
    HttpRequest(const string &method, const string &url,
                const string &body = "", const string &version = "1.0");
    ~HttpRequest();

    string getUrl() const;
    void setUrl(const string& url);
    void setMethod(const string& method);
    void setMethod(Method method);
    string getMethod() const;
    string toString() const;
    string getPath() const;
};

#endif // HTTPREQUEST_H
