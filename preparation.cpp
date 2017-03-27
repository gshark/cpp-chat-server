#include "preparation.h"

#include <fstream>


void preparation::prepareFile(std::string name, std::string data) {
    std::fstream fs(name, std::fstream::out);
    fs << data;
    fs.close();
}

void preparation::prepare() {
    preparation::prepareFile(preparation::INDEX_FILE_NAME, preparation::INDEX_DATA);
    preparation::prepareFile(preparation::JQUERY_FILE_NAME, preparation::JQUERY_DATA);
    preparation::prepareFile(preparation::SCRIPT_FILE_NAME, preparation::SCRIPT_DATA);
}
