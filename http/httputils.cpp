#include "httputils.h"
#include <cassert>

string HttpUtils::toLower(string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

string HttpUtils::transformRoute(string route) {
    if (route == "") {
        return route;
    }
    route = toLower(route);
    QUrl url(route.c_str());
    QString qurl = url.path();
    if (!qurl.isEmpty() && qurl[qurl.size() - 1] == '/' && qurl.size() != 1) {
        qurl.remove(qurl.size() - 1, 1);
    }
    route = qurl.toStdString();
    return route;
}
