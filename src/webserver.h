#pragma once
#include "database.h"
#include "logger.h"  // Добавляем логгер
#include <crow.h>
#include <string>
#include <memory>

class WebServer {
private:
    std::unique_ptr<Database> db;
    crow::SimpleApp app;
    int port;
    
    void setupRoutes();
    void setupLogging();  // Новая функция для настройки логирования
    
public:
    WebServer(const std::string& config_file);
    void run();
};
