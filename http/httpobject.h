#ifndef HTTPOBJECT_H
#define HTTPOBJECT_H

#include <string>
#include <vector>
#include <sstream>
#include <tcp/tcpsocket.h>
#include <memory>

using std::string;

class HttpObject {
public:
    enum CreationMode { DYNAMIC, STATIC };
private:
    typedef std::vector<std::pair<string, string>> HeadersContainer;

    bool mHasBody;
    string allData;
    size_t index;
    CreationMode mode;

    //string toLower(string s) const;
    string trim(const string &s);
protected:
    virtual void parseFirstLine(const string &line) = 0;

    string version;
    string body;
    HeadersContainer headers;
public:
    const string PREFIX = "HTTP/";
    const size_t PREFIX_LENGTH = PREFIX.size();

    HttpObject(CreationMode mode);
    HttpObject(const string &body, const string &version);
    virtual ~HttpObject();

    bool append(const string &data);
    void commit();
    bool hasBody();

    virtual void setUrl(const string &url) = 0;
    virtual string getUrl() const = 0;
    virtual string toString() const;

    string getVersion() const;
    void addHeader(const string& key, const string& val);
    void addHeaders(const std::vector<std::pair <string, string>> &headers);
    void setHeader(const string &key, const string &val);
    string header(const string &key);
    HeadersContainer getHeaders() const;
    void setBody(const string &message);
    void setBody(const char *message);
    string getBody() const;
    bool isKeepAlive() const;
    string findHeader(string &key) const;
    string findHeader(const char *key) const;

    string getHost() const;
    int contentLength() const;
};

#endif // HTTPOBJECT_H
