#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace Logger {

    void info(const std::string &message);
    void error(const std::string &message);
    void status(const std::string &message);
    void status(int exitCode);
}

#endif // LOGGER_H
