#include "httpobject.h"
#include "httputils.h"


#include <cstring>
#include <algorithm>

HttpObject::HttpObject(HttpObject::CreationMode mode) :
    mHasBody(false),
    index(0),
    mode(mode) {}

HttpObject::HttpObject(const string &body, const string &version) {
    this->body = body;
    this->version = version;
    setHeader("Content-Length", std::to_string(body.size()));
}

HttpObject::~HttpObject() {
}

/*string HttpObject::toLower(string s) const {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}*/

string HttpObject::trim(const string &s) {
    if (s.size() == 0) {
        return "";
    }
    size_t i = 0;
    size_t j = s.size() - 1;
    while (i < s.size() && s[i] == ' ') {
        ++i;
    }
    while (j >= i && s[j] == ' ') {
        --j;
    }
    if (i > j) {
        return "";
    }
    return s.substr(i, j - i + 1);
}

bool isNewLine(const string &data, size_t index) {
    return (index >= 1 && data[index - 1] == '\n') ||
           (index >= 2 && data[index - 1] == '\r' && data[index - 2] == '\n');
}

bool HttpObject::append(const string &data) {
    if (mode == STATIC) {
        return false;
    }

    if (mHasBody) {
        body += data;
    } else {
        allData += data;
        for (; index != allData.size(); ++index) {
            if (allData[index] == '\n' && isNewLine(data, index)) {
                break;
            }
        }
        if (index == allData.size()) {
            return true;
        }
        mHasBody = true;
        if (index + 1 < allData.size()) {
            body = allData.substr(index + 1, allData.size() - index - 1);
            allData.erase(index + 1, allData.size() - index - 1);
        }

        std::stringstream stream(allData);
        string line;
        getline(stream, line);
        parseFirstLine(line);
        while (getline(stream, line)) {
            size_t position = line.find(":");
            if (position == string::npos) {
                continue;
            }
            string key = HttpUtils::toLower(trim(line.substr(0, position)));
            string value = trim(line.substr(position + 1, line.size() - position - 1));
            headers.push_back(std::make_pair(key, value));
        }
    }
    return true;
}

void HttpObject::commit() {
    mode = STATIC;
}

bool HttpObject::hasBody() {
    return mHasBody;
}

string HttpObject::getVersion() const {
    return version;
}

void HttpObject::addHeader(const string &key, const string &value) {
    headers.push_back(std::make_pair(HttpUtils::toLower(key), HttpUtils::toLower(value)));
}

void HttpObject::setHeader(const string &key, const string &value) {
    string lowerKey = HttpUtils::toLower(key);
    for (size_t i = 0; i != headers.size(); ++i) {
        if (headers[i].first == lowerKey) {
            headers[i].second = value;
            return;
        }
    }
    headers.push_back(std::make_pair(lowerKey, HttpUtils::toLower(value)));
}

void HttpObject::addHeaders(const std::vector<std::pair<string, string>> &headers) {
    for (size_t i = 0; i != headers.size(); ++i) {
        this->headers.push_back(std::make_pair(HttpUtils::toLower(headers[i].first), headers[i].second));
    }
}

string HttpObject::header(const string &key) {
    string lowerKey = HttpUtils::toLower(key);
    for (size_t i = 0; i != headers.size(); ++i) {
        if (headers[i].first == lowerKey) {
            return headers[i].second;
        }
    }
    return "";
}

HttpObject::HeadersContainer HttpObject::getHeaders() const {
    return headers;
}

void HttpObject::setBody(const string &body) {
    setBody(body.c_str());
    setHeader("Content-Length", std::to_string(body.size()));
}

void HttpObject::setBody(const char *body) {
    this->body = body;
}

string HttpObject::getBody() const {
    return body;
}

string HttpObject::findHeader(string &key) const {
    for (size_t i = 0; i != headers.size(); ++i) {
        if (headers[i].first == key) {
            return headers[i].second;
        }
    }
    return "";
}

string HttpObject::findHeader(const char *key) const {
    string sKey = key;
    return findHeader(sKey);
}

string HttpObject::getHost() const {
    return HttpUtils::toLower(findHeader("host"));
}

int HttpObject::contentLength() const {
    string result = findHeader("content-length");
    if (result.size() == 0) {
        return 0;
    }
    return atoi(result.c_str());
}

bool HttpObject::isKeepAlive() const {
    return HttpUtils::toLower(findHeader("connection")) != "keep-alive";
}

string HttpObject::toString() const {
    string result = "";
    for (auto &element : headers) {
        result += element.first + ": " + element.second + '\n';
    }
    result += '\n';
    result += body;
    return result;
}
