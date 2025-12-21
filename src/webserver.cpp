#include "webserver.h"
#include <fstream>
#include <iostream>

WebServer::WebServer(const std::string& config_file) : port(8080) {
    // Чтение конфигурации
    std::ifstream config_stream(config_file);
    if (!config_stream) {
        std::cerr << "Cannot open config file: " << config_file << std::endl;
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
        
        db = std::make_unique<Database>(conn_str);
        
        if (!db->connect()) {
            std::cerr << "Failed to connect to database" << std::endl;
            return;
        }
        
        port = config["server"]["port"].get<int>();
        
        setupRoutes();
        
    } catch (const std::exception& e) {
        std::cerr << "Config error: " << e.what() << std::endl;
    }
}

void WebServer::setupRoutes() {
    // Статические файлы - правильный путь
    CROW_ROUTE(app, "/")
    ([]() {
        std::ifstream file("www/index.html");
        if (!file) {
            // Альтернативный путь
            file.open("../www/index.html");
            if (!file) {
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
        std::string path = "www/" + filename;
        std::ifstream file(path);
        
        if (!file) {
            // Попробуем альтернативный путь
            path = "../www/" + filename;
            file.open(path);
            
            if (!file) {
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
        bool connected = db->testConnection();
        
        json response;
        response["database_connected"] = connected;
        response["timestamp"] = std::time(nullptr);
        
        crow::response res;
        res.set_header("Content-Type", "application/json; charset=utf-8");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.body = response.dump();
        return res;
    });
    
    // API: Получение всех устройств
    CROW_ROUTE(app, "/api/devices")
    .methods("GET"_method)
    ([this]() {
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
        return res;
    });
    
    // API: Добавление нового устройства
    CROW_ROUTE(app, "/api/devices")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            Device device;
            device.name = body["name"].get<std::string>();
            device.model = body["model"].get<std::string>();
            device.purchase_date = body["purchase_date"].get<std::string>();
            device.status = body["status"].get<std::string>();
            
            bool success = db->addDevice(device);
            
            json response;
            response["success"] = success;
            
            crow::response res;
            res.set_header("Content-Type", "application/json; charset=utf-8");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.body = response.dump();
            return res;
        } catch (const std::exception& e) {
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
        return res;
    });
    
    // API: Получение истории обслуживания (детализированная с JOIN)
    CROW_ROUTE(app, "/api/service-history")
    .methods("GET"_method)
    ([this]() {
        auto history = db->getDetailedServiceHistory();
        
        crow::response res;
        res.set_header("Content-Type", "application/json; charset=utf-8");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.body = history.dump();
        return res;
    });
    
    // API: Добавление записи обслуживания
    CROW_ROUTE(app, "/api/service-history")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            ServiceRecord record;
            record.device_id = body["device_id"].get<int>();
            record.service_id = body["service_id"].get<int>();
            record.service_date = body["service_date"].get<std::string>();
            record.cost = body["cost"].get<double>();
            record.notes = body["notes"].get<std::string>();
            record.next_due_date = body["next_due_date"].get<std::string>();
            
            bool success = db->addServiceRecord(record);
            
            json response;
            response["success"] = success;
            
            crow::response res;
            res.set_header("Content-Type", "application/json; charset=utf-8");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.body = response.dump();
            return res;
        } catch (const std::exception& e) {
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
        return res;
    });
}

void WebServer::run() {
    std::cout << "Starting server on port " << port << std::endl;
    app.port(port).multithreaded().run();
}