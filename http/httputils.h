#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <tcp/tcpsocket.h>
#include <http/httpobject.h>
#include <http/httprequest.h>
#include <memory>

#include <QUrl>

namespace HttpUtils {
    string toLower(string s);
    string transformRoute(string route);
}

#endif // HTTPUTILS_H
