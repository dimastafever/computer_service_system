#pragma once
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Device {
    int id;
    std::string name;
    std::string model;
    std::string purchase_date;
    std::string status;
};

struct ServiceType {
    int id;
    std::string name;
    int recommended_interval_months;
    double standard_cost;
};

struct ServiceRecord {
    int id;
    int device_id;
    int service_id;
    std::string service_date;
    double cost;
    std::string notes;
    std::string next_due_date;
};

class Database {
private:
    std::unique_ptr<pqxx::connection> conn;
    
public:
    Database(const std::string& conn_str);
    ~Database();
    
    bool connect();
    bool testConnection();
    
    // Устройства
    std::vector<Device> getAllDevices();
    bool addDevice(const Device& device);
    bool updateDevice(int id, const Device& device);
    bool deleteDevice(int id);
    
    // Типы услуг
    std::vector<ServiceType> getAllServiceTypes();
    bool addServiceType(const ServiceType& type);
    bool updateServiceType(int id, const ServiceType& type);
    bool deleteServiceType(int id);
    
    // История обслуживания
    std::vector<ServiceRecord> getAllServiceRecords();
    bool addServiceRecord(const ServiceRecord& record);
    bool updateServiceRecord(int id, const ServiceRecord& record);
    bool deleteServiceRecord(int id);
    
    // Получение детализированной истории с JOIN
    json getDetailedServiceHistory();
};