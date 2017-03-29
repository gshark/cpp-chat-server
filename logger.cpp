#include "logger.h"


void Logger::info(const std::string &message) {
    fprintf(stderr, "INFO: %s\n", message.c_str());
}

void Logger::error(const std::string &message) {
    fprintf(stderr, "ERROR: %s\n", message.c_str());
}

void Logger::status(const std::string &message) {
    fprintf(stderr, "Status: %s\n", message.c_str());
}


//TODO Logger класс со статик функциями, приватным конструтором и возможно namespace
