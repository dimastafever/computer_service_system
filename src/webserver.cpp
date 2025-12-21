#include "webserver.h"
#include <fstream>
#include <iostream>

WebServer::WebServer(const std::string& config_file) : port(8080) {
    // Настраиваем логирование
    setupLogging();
    
    LOG_INFO("Initializing WebServer with config file: " + config_file);
    
    // Чтение конфигурации
    std::ifstream config_stream(config_file);
    if (!config_stream) {
        LOG_ERROR("Cannot open config file: " + config_file);
        return;
    }
    
    try {
        json config;
        config_stream >> config;
        
        // Конфигурация базы данных
        std::string conn_str = 
            "host=" + config["database"]["host"].get<std::string>() + " " +
            "port=" + std::to_string(config["database"]["port"].get<int>()) + " " +
            "dbname=" + config["database"]["dbname"].get<std::string>() + " " +
            "user=" + config["database"]["user"].get<std::string>() + " " +
            "password=" + config["database"]["password"].get<std::string>();
        
        LOG_INFO("Database connection string prepared");
        
        db = std::make_unique<Database>(conn_str);
        
        if (!db->connect()) {
            LOG_ERROR("Failed to connect to database");
            return;
        }
        
        LOG_INFO("Database connected successfully");
        
        port = config["server"]["port"].get<int>();
        
        setupRoutes();
        
        LOG_INFO("WebServer initialized successfully on port " + std::to_string(port));
        
    } catch (const std::exception& e) {
        LOG_ERROR("Config error: " + std::string(e.what()));
    }
}

void WebServer::setupLogging() {
    auto& logger = Logger::getInstance();
    logger.setLogFile("server.log");
    logger.setLogToConsole(true);
    logger.setMinLevel(LogLevel::DEBUG);
    LOG_INFO("Logging system initialized");
}

void WebServer::setupRoutes() {
    LOG_DEBUG("Setting up routes...");
    
    // Статические файлы - правильный путь
    CROW_ROUTE(app, "/")
    ([]() {
        LOG_DEBUG("Request to root path");
        std::ifstream file("www/index.html");
        if (!file) {
            // Альтернативный путь
            LOG_DEBUG("Trying alternative path for index.html");
            file.open("../www/index.html");
            if (!file) {
                LOG_WARNING("Index file not found");
                return crow::response(404, "Index file not found");
            }
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        crow::response res;
        res.set_header("Content-Type", "text/html; charset=utf-8");
        res.body = content;
        return res;
    });
    
    // Статические файлы CSS, JS и т.д.
    CROW_ROUTE(app, "/<string>")
    ([](const std::string& filename) {
        LOG_DEBUG("Request for static file: " + filename);
        std::string path = "www/" + filename;
        std::ifstream file(path);
        
        if (!file) {
            // Попробуем альтернативный путь
            LOG_DEBUG("Trying alternative path for: " + filename);
            path = "../www/" + filename;
            file.open(path);
            
            if (!file) {
                LOG_WARNING("File not found: " + filename);
                return crow::response(404, "File not found: " + filename);
            }
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        crow::response res;
        
        // Определение Content-Type по расширению
        if (filename.find(".css") != std::string::npos) {
            res.set_header("Content-Type", "text/css");
        } else if (filename.find(".js") != std::string::npos) {
            res.set_header("Content-Type", "application/javascript");
        } else if (filename.find(".png") != std::string::npos) {
            res.set_header("Content-Type", "image/png");
        } else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos) {
            res.set_header("Content-Type", "image/jpeg");
        } else {
            res.set_header("Content-Type", "text/html");
        }
        
        res.body = content;
        return res;
    });
    
    // API: Тест подключения к БД
    CROW_ROUTE(app, "/api/test-db")
    ([this]() {
        LOG_DEBUG("API request: /api/test-db");
        bool connected = db->testConnection();
        
        json response;
        response["database_connected"] = connected;
        response["timestamp"] = std::time(nullptr);
        
        crow::response res;
        res.set_header("Content-Type", "application/json; charset=utf-8");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.body = response.dump();
        
        if (connected) {
            LOG_DEBUG("Database test: connected");
        } else {
            LOG_WARNING("Database test: not connected");
        }
        
        return res;
    });
    
    // API: Получение всех устройств
    CROW_ROUTE(app, "/api/devices")
    .methods("GET"_method)
    ([this]() {
        LOG_DEBUG("API request: GET /api/devices");
        auto devices = db->getAllDevices();
        json result = json::array();
        
        for (const auto& device : devices) {
            json j;
            j["id"] = device.id;
            j["name"] = device.name;
            j["model"] = device.model;
            j["purchase_date"] = device.purchase_date;
            j["status"] = device.status;
            result.push_back(j);
        }
        
        crow::response res;
        res.set_header("Content-Type", "application/json; charset=utf-8");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.body = result.dump();
        
        LOG_DEBUG("Returning " + std::to_string(devices.size()) + " devices");
        return res;
    });
    
    // API: Добавление нового устройства
    CROW_ROUTE(app, "/api/devices")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        LOG_DEBUG("API request: POST /api/devices");
        try {
            auto body = json::parse(req.body);
            Device device;
            device.name = body["name"].get<std::string>();
            device.model = body["model"].get<std::string>();
            device.purchase_date = body["purchase_date"].get<std::string>();
            device.status = body["status"].get<std::string>();
            
            LOG_INFO("Adding new device: " + device.name);
            
            bool success = db->addDevice(device);
            
            json response;
            response["success"] = success;
            
            crow::response res;
            res.set_header("Content-Type", "application/json; charset=utf-8");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.body = response.dump();
            
            if (success) {
                LOG_INFO("Device added successfully: " + device.name);
            } else {
                LOG_ERROR("Failed to add device: " + device.name);
            }
            
            return res;
        } catch (const std::exception& e) {
            LOG_ERROR("Error adding device: " + std::string(e.what()));
            
            json response;
            response["success"] = false;
            response["error"] = e.what();
            
            crow::response res(400);
            res.set_header("Content-Type", "application/json; charset=utf-8");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.body = response.dump();
            return res;
        }
    });
    
    // API: Получение всех типов услуг
    CROW_ROUTE(app, "/api/service-types")
    .methods("GET"_method)
    ([this]() {
        LOG_DEBUG("API request: GET /api/service-types");
        auto types = db->getAllServiceTypes();
        json result = json::array();
        
        for (const auto& type : types) {
            json j;
            j["id"] = type.id;
            j["name"] = type.name;
            j["recommended_interval_months"] = type.recommended_interval_months;
            j["standard_cost"] = type.standard_cost;
            result.push_back(j);
        }
        
        crow::response res;
        res.set_header("Content-Type", "application/json; charset=utf-8");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.body = result.dump();
        
        LOG_DEBUG("Returning " + std::to_string(types.size()) + " service types");
        return res;
    });
    
    // API: Получение истории обслуживания (детализированная с JOIN)
    CROW_ROUTE(app, "/api/service-history")
    .methods("GET"_method)
    ([this]() {
        LOG_DEBUG("API request: GET /api/service-history");
        auto history = db->getDetailedServiceHistory();
        
        crow::response res;
        res.set_header("Content-Type", "application/json; charset=utf-8");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.body = history.dump();
        
        LOG_DEBUG("Returning service history with " + std::to_string(history.size()) + " records");
        return res;
    });
    
    // API: Добавление записи обслуживания
    CROW_ROUTE(app, "/api/service-history")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        LOG_DEBUG("API request: POST /api/service-history");
        try {
            auto body = json::parse(req.body);
            ServiceRecord record;
            record.device_id = body["device_id"].get<int>();
            record.service_id = body["service_id"].get<int>();
            record.service_date = body["service_date"].get<std::string>();
            record.cost = body["cost"].get<double>();
            record.notes = body["notes"].get<std::string>();
            record.next_due_date = body["next_due_date"].get<std::string>();
            
            LOG_INFO("Adding service record for device ID: " + std::to_string(record.device_id));
            
            bool success = db->addServiceRecord(record);
            
            json response;
            response["success"] = success;
            
            crow::response res;
            res.set_header("Content-Type", "application/json; charset=utf-8");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.body = response.dump();
            
            if (success) {
                LOG_INFO("Service record added successfully");
            } else {
                LOG_ERROR("Failed to add service record");
            }
            
            return res;
        } catch (const std::exception& e) {
            LOG_ERROR("Error adding service record: " + std::string(e.what()));
            
            json response;
            response["success"] = false;
            response["error"] = e.what();
            
            crow::response res(400);
            res.set_header("Content-Type", "application/json; charset=utf-8");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.body = response.dump();
            return res;
        }
    });
    
    // API: Получение всех записей обслуживания (простой вариант)
    CROW_ROUTE(app, "/api/service-records")
    .methods("GET"_method)
    ([this]() {
        LOG_DEBUG("API request: GET /api/service-records");
        auto records = db->getAllServiceRecords();
        json result = json::array();
        
        for (const auto& record : records) {
            json j;
            j["id"] = record.id;
            j["device_id"] = record.device_id;
            j["service_id"] = record.service_id;
            j["service_date"] = record.service_date;
            j["cost"] = record.cost;
            j["notes"] = record.notes;
            j["next_due_date"] = record.next_due_date;
            result.push_back(j);
        }
        
        crow::response res;
        res.set_header("Content-Type", "application/json; charset=utf-8");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.body = result.dump();
        
        LOG_DEBUG("Returning " + std::to_string(records.size()) + " service records");
        return res;
    });
    
    LOG_INFO("Routes setup completed");
}

void WebServer::run() {
    LOG_INFO("Starting server on port " + std::to_string(port));
    std::cout << "Starting server on port " << port << std::endl;
    
    try {
        app.port(port).multithreaded().run();
    } catch (const std::exception& e) {
        LOG_ERROR("Server runtime error: " + std::string(e.what()));
        throw;
    }
}
