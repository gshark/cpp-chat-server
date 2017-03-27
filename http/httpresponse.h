#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <http/httpobject.h>

class HttpResponse : public HttpObject {
private:
    int statusCode;
    string statusText;
    string url;
protected:
    void parseFirstLine(const string &line);
public:
    HttpResponse(CreationMode mode = STATIC);
    HttpResponse(int statusCode, const string &statusText,
                 const string &version="1.0", const string &message = "");
    ~HttpResponse();

    void setUrl(const string &url);
    string getUrl() const;
    void setStatusCode(int statusCode);
    int getStatusCode() const;
    void setStatusText(const string &rph);
    string getStatusText() const;
    string toString() const;
};

#endif // HTTPRESPONSE_H
