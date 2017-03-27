#include <executor.h>
#include <preparation.h>
#include <chat/chatserver.h>
#include <cassert>

int main(int, char**) {
    //preparation::prepare();
    Executor executor;
    ChatServer server(&executor);
    //Logger logger;
    try {
        Executor executor;
        ChatServer server(&executor);
        server.start(2390);
        return executor.execute();
    } catch (std::exception const &e) {
        Logger::error("Starting server failed: " + std::string(e.what()));
        return 1;
    }
}

//TODO для всех noncopy классов запретить копирование, чтоб работала из любого каталога
//ld -r -b binary -> google
//Raw string literals
