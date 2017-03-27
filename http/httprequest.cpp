#include "httprequest.h"
#include <QUrl>

HttpRequest::HttpRequest(CreationMode mode) :
    HttpObject(mode) {}

string HttpRequest::getMethod(Method method) {
    std::vector<int> methods = {HEAD, GET, PUT, POST, DELETE};
    const string methodsDescription[] = {"HEAD", "GET", "PUT", "POST", "DELETE"};
    for (size_t i = 0; i != methods.size(); ++i) {
        if (method == methods[i]) {
            return methodsDescription[i];
        }
    }
    return "";
}

HttpRequest::HttpRequest(Method method, const string &url, const string &body,
                         const string &version) :
    HttpObject(body, version) {
    this->method = getMethod(method);
    string h;
    splitUrl(url, h, path);
    addHeader("Host", h);
}

HttpRequest::HttpRequest(const string &method, const string &url,
                         const string &body, const string &version) :
    HttpObject(body, version) {
    this->method = method;
    string host;
    splitUrl(url, host, path);
    addHeader("Host", host);
}

HttpRequest::~HttpRequest() {
}

void HttpRequest::parseFirstLine(const string &line) {
    std::stringstream stream(line);
    stream >> method >> path >> version;
    version.erase(0, PREFIX_LENGTH);
}

void HttpRequest::setUrl(const string &url) {
    string host;
    splitUrl(url, host, path);
    addHeader("Host", host);
}

string HttpRequest::getUrl() const {
    return getHost() + getPath();
}

string HttpRequest::getPath() const {
    return path;
}

void HttpRequest::splitUrl(string url, string &host, string &path) {
    if (!QString(url.c_str()).startsWith("http://", Qt::CaseInsensitive)) {
        url = "http://" + url;
    }
    QUrl qurl(url.c_str());
    host = qurl.host().toStdString();
    path = qurl.path().toStdString();
    if (path == "") {
        path = "/";
    }
}

void HttpRequest::setMethod(const string &method) {
    this->method = method;
}

void HttpRequest::setMethod(Method method) {
    this->method = getMethod(method);
}

string HttpRequest::getMethod() const {
    return method;
}

string HttpRequest::toString() const {
    string result = getMethod() + " " + getPath() +
                    ' ' + PREFIX + getVersion() + '\n';
    return result + HttpObject::toString();
}
