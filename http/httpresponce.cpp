#include "httpresponse.h"

void HttpResponse::parseFirstLine(const string &line) {
    std::stringstream stream(line);
    stream >> version >> statusCode >> statusText;
    version.erase(0, PREFIX_LENGTH);
}

HttpResponse::HttpResponse(CreationMode mode) :
    HttpObject(mode) {}

HttpResponse::HttpResponse(int statusCode,
                           const string &statusText,
                           const string &version,
                           const string &body) :
    HttpObject(body, version) {
    this->statusCode = statusCode;
    this->statusText = statusText;
}

HttpResponse::~HttpResponse() {
}

void HttpResponse::setUrl(const string &url) {
    this->url = url;
}

string HttpResponse::getUrl() const {
    return url;
}

void HttpResponse::setStatusCode(int statusCode) {
    this->statusCode = statusCode;
}

int HttpResponse::getStatusCode() const {
    return statusCode;
}

void HttpResponse::setStatusText(const string &statusText) {
    this->statusText = statusText;
}

string HttpResponse::getStatusText() const {
    return statusText;
}

string HttpResponse::toString() const {
    string result = "HTTP/" + getVersion() + " ";
    result += std::to_string(statusCode) + " " + statusText + '\n' + HttpObject::toString();
    return result;
}
