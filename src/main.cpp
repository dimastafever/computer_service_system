#include "webserver.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string config_file = "config.json";
    
    if (argc > 1) {
        config_file = argv[1];
    }
    
    try {
        WebServer server(config_file);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}