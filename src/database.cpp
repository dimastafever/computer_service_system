#include "database.h"
#include "logger.h"  // Добавляем логгер
#include <iostream>

Database::Database(const std::string& conn_str) {
    LOG_DEBUG("Creating database connection");
    try {
        conn = std::make_unique<pqxx::connection>(conn_str);
        if (conn->is_open()) {
            LOG_INFO("Connected to database successfully");
            std::cout << "Connected to database successfully" << std::endl;
        } else {
            LOG_ERROR("Failed to connect to database");
            std::cerr << "Failed to connect to database" << std::endl;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Database connection error: " + std::string(e.what()));
        std::cerr << "Database connection error: " << e.what() << std::endl;
    }
}

Database::~Database() {
    LOG_DEBUG("Database connection closing");
    // Удаляем вызов conn->close() - соединение закроется автоматически
    // При уничтожении unique_ptr, деструктор pqxx::connection вызовется автоматически
}

bool Database::connect() {
    bool connected = conn && conn->is_open();
    if (!connected) {
        LOG_WARNING("Database connection check failed");
    }
    return connected;
}

bool Database::testConnection() {
    LOG_DEBUG("Testing database connection");
    try {
        pqxx::work txn(*conn);
        txn.exec("SELECT 1");
        LOG_DEBUG("Database connection test successful");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Database connection test failed: " + std::string(e.what()));
        std::cerr << "Test connection failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Device> Database::getAllDevices() {
    LOG_DEBUG("Getting all devices from database");
    std::vector<Device> devices;
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec(
            "SELECT device_id, name, model, purchase_date, status FROM Devices ORDER BY device_id"
        );
        
        for (const auto& row : result) {
            Device d;
            d.id = row[0].as<int>();
            d.name = row[1].as<std::string>();
            d.model = row[2].as<std::string>("");
            d.purchase_date = row[3].as<std::string>("");
            d.status = row[4].as<std::string>("active");
            devices.push_back(d);
        }
        txn.commit();
        
        LOG_DEBUG("Retrieved " + std::to_string(devices.size()) + " devices from database");
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting devices: " + std::string(e.what()));
        std::cerr << "Error getting devices: " << e.what() << std::endl;
    }
    return devices;
}

bool Database::addDevice(const Device& device) {
    LOG_INFO("Adding device to database: " + device.name);
    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO Devices (name, model, purchase_date, status) VALUES ($1, $2, $3, $4)",
            device.name,
            device.model,
            device.purchase_date.empty() ? nullptr : device.purchase_date.c_str(),
            device.status
        );
        txn.commit();
        
        LOG_INFO("Device added successfully: " + device.name);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error adding device '" + device.name + "': " + std::string(e.what()));
        std::cerr << "Error adding device: " << e.what() << std::endl;
        return false;
    }
}

// Реализация недостающих методов для Device
bool Database::updateDevice(int id, const Device& device) {
    LOG_INFO("Updating device ID " + std::to_string(id) + ": " + device.name);
    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "UPDATE Devices SET name=$1, model=$2, purchase_date=$3, status=$4 WHERE device_id=$5",
            device.name,
            device.model,
            device.purchase_date.empty() ? nullptr : device.purchase_date.c_str(),
            device.status,
            id
        );
        txn.commit();
        
        LOG_INFO("Device updated successfully: " + device.name);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error updating device ID " + std::to_string(id) + ": " + std::string(e.what()));
        std::cerr << "Error updating device: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteDevice(int id) {
    LOG_WARNING("Deleting device ID " + std::to_string(id));
    try {
        pqxx::work txn(*conn);
        txn.exec_params("DELETE FROM Devices WHERE device_id=$1", id);
        txn.commit();
        
        LOG_INFO("Device deleted successfully: ID " + std::to_string(id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error deleting device ID " + std::to_string(id) + ": " + std::string(e.what()));
        std::cerr << "Error deleting device: " << e.what() << std::endl;
        return false;
    }
}

std::vector<ServiceType> Database::getAllServiceTypes() {
    LOG_DEBUG("Getting all service types from database");
    std::vector<ServiceType> types;
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec(
            "SELECT service_id, name, recommended_interval_months, standard_cost FROM Service_Types ORDER BY service_id"
        );
        
        for (const auto& row : result) {
            ServiceType st;
            st.id = row[0].as<int>();
            st.name = row[1].as<std::string>();
            st.recommended_interval_months = row[2].as<int>(0);
            st.standard_cost = row[3].as<double>(0.0);
            types.push_back(st);
        }
        txn.commit();
        
        LOG_DEBUG("Retrieved " + std::to_string(types.size()) + " service types from database");
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting service types: " + std::string(e.what()));
        std::cerr << "Error getting service types: " << e.what() << std::endl;
    }
    return types;
}

// Реализация недостающих методов для ServiceType
bool Database::addServiceType(const ServiceType& type) {
    LOG_INFO("Adding service type: " + type.name);
    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO Service_Types (name, recommended_interval_months, standard_cost) VALUES ($1, $2, $3)",
            type.name,
            type.recommended_interval_months,
            type.standard_cost
        );
        txn.commit();
        
        LOG_INFO("Service type added successfully: " + type.name);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error adding service type '" + type.name + "': " + std::string(e.what()));
        std::cerr << "Error adding service type: " << e.what() << std::endl;
        return false;
    }
}

bool Database::updateServiceType(int id, const ServiceType& type) {
    LOG_INFO("Updating service type ID " + std::to_string(id) + ": " + type.name);
    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "UPDATE Service_Types SET name=$1, recommended_interval_months=$2, standard_cost=$3 WHERE service_id=$4",
            type.name,
            type.recommended_interval_months,
            type.standard_cost,
            id
        );
        txn.commit();
        
        LOG_INFO("Service type updated successfully: " + type.name);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error updating service type ID " + std::to_string(id) + ": " + std::string(e.what()));
        std::cerr << "Error updating service type: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteServiceType(int id) {
    LOG_WARNING("Deleting service type ID " + std::to_string(id));
    try {
        pqxx::work txn(*conn);
        txn.exec_params("DELETE FROM Service_Types WHERE service_id=$1", id);
        txn.commit();
        
        LOG_INFO("Service type deleted successfully: ID " + std::to_string(id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error deleting service type ID " + std::to_string(id) + ": " + std::string(e.what()));
        std::cerr << "Error deleting service type: " << e.what() << std::endl;
        return false;
    }
}

std::vector<ServiceRecord> Database::getAllServiceRecords() {
    LOG_DEBUG("Getting all service records from database");
    std::vector<ServiceRecord> records;
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec(
            "SELECT record_id, device_id, service_id, service_date, cost, notes, next_due_date "
            "FROM Service_History ORDER BY service_date DESC"
        );
        
        for (const auto& row : result) {
            ServiceRecord sr;
            sr.id = row[0].as<int>();
            sr.device_id = row[1].as<int>();
            sr.service_id = row[2].as<int>();
            sr.service_date = row[3].as<std::string>();
            sr.cost = row[4].as<double>(0.0);
            sr.notes = row[5].as<std::string>("");
            sr.next_due_date = row[6].as<std::string>("");
            records.push_back(sr);
        }
        txn.commit();
        
        LOG_DEBUG("Retrieved " + std::to_string(records.size()) + " service records from database");
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting service records: " + std::string(e.what()));
        std::cerr << "Error getting service records: " << e.what() << std::endl;
    }
    return records;
}

bool Database::addServiceRecord(const ServiceRecord& record) {
    LOG_INFO("Adding service record for device ID " + std::to_string(record.device_id));
    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO Service_History (device_id, service_id, service_date, cost, notes, next_due_date) "
            "VALUES ($1, $2, $3, $4, $5, $6)",
            record.device_id,
            record.service_id,
            record.service_date,
            record.cost,
            record.notes,
            record.next_due_date.empty() ? nullptr : record.next_due_date.c_str()
        );
        txn.commit();
        
        LOG_INFO("Service record added successfully for device ID " + std::to_string(record.device_id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error adding service record: " + std::string(e.what()));
        std::cerr << "Error adding service record: " << e.what() << std::endl;
        return false;
    }
}

// Реализация недостающих методов для ServiceRecord
bool Database::updateServiceRecord(int id, const ServiceRecord& record) {
    LOG_INFO("Updating service record ID " + std::to_string(id));
    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "UPDATE Service_History SET device_id=$1, service_id=$2, service_date=$3, cost=$4, notes=$5, next_due_date=$6 WHERE record_id=$7",
            record.device_id,
            record.service_id,
            record.service_date,
            record.cost,
            record.notes,
            record.next_due_date.empty() ? nullptr : record.next_due_date.c_str(),
            id
        );
        txn.commit();
        
        LOG_INFO("Service record updated successfully: ID " + std::to_string(id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error updating service record ID " + std::to_string(id) + ": " + std::string(e.what()));
        std::cerr << "Error updating service record: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteServiceRecord(int id) {
    LOG_WARNING("Deleting service record ID " + std::to_string(id));
    try {
        pqxx::work txn(*conn);
        txn.exec_params("DELETE FROM Service_History WHERE record_id=$1", id);
        txn.commit();
        
        LOG_INFO("Service record deleted successfully: ID " + std::to_string(id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error deleting service record ID " + std::to_string(id) + ": " + std::string(e.what()));
        std::cerr << "Error deleting service record: " << e.what() << std::endl;
        return false;
    }
}

json Database::getDetailedServiceHistory() {
    LOG_DEBUG("Getting detailed service history with JOIN");
    json result = json::array();
    try {
        pqxx::work txn(*conn);
        pqxx::result rows = txn.exec(
            "SELECT "
            "sh.record_id, "
            "d.name as device_name, "
            "d.model, "
            "st.name as service_name, "
            "sh.service_date, "
            "sh.cost, "
            "sh.notes, "
            "sh.next_due_date "
            "FROM Service_History sh "
            "JOIN Devices d ON sh.device_id = d.device_id "
            "JOIN Service_Types st ON sh.service_id = st.service_id "
            "ORDER BY sh.service_date DESC"
        );
        
        for (const auto& row : rows) {
            json record;
            record["record_id"] = row[0].as<int>();
            record["device_name"] = row[1].as<std::string>();
            record["model"] = row[2].as<std::string>("");
            record["service_name"] = row[3].as<std::string>();
            record["service_date"] = row[4].as<std::string>();
            record["cost"] = row[5].as<double>(0.0);
            record["notes"] = row[6].as<std::string>("");
            record["next_due_date"] = row[7].as<std::string>("");
            result.push_back(record);
        }
        txn.commit();
        
        LOG_DEBUG("Retrieved detailed service history with " + std::to_string(result.size()) + " records");
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting detailed history: " + std::string(e.what()));
        std::cerr << "Error getting detailed history: " << e.what() << std::endl;
    }
    return result;
}
