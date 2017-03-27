#include "httpmatcher.h"

#include <http/httputils.h>

HttpMatcher::HttpMatcher(const string &method, const string &url) :
    method(method), url(url) {
    normalize();
}

bool HttpMatcher::match(const HttpRequest &request) {
    string methodLower = HttpUtils::toLower(request.getMethod());
    string pathRoute = HttpUtils::transformRoute(request.getPath());
    return methodLower == method && pathRoute == url;
}

void HttpMatcher::normalize() {
    method = HttpUtils::toLower(method);
    url = HttpUtils::transformRoute(url);
}

string HttpMatcher::getMethod() {
    return method;
}

string HttpMatcher::getUrl() {
    return url;
}
