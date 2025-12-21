#include "database.h"
#include <iostream>

Database::Database(const std::string& conn_str) {
    try {
        conn = std::make_unique<pqxx::connection>(conn_str);
        if (conn->is_open()) {
            std::cout << "Connected to database successfully" << std::endl;
        } else {
            std::cerr << "Failed to connect to database" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Database connection error: " << e.what() << std::endl;
    }
}

Database::~Database() {
    // Удаляем вызов conn->close() - соединение закроется автоматически
    // При уничтожении unique_ptr, деструктор pqxx::connection вызовется автоматически
}

bool Database::connect() {
    return conn && conn->is_open();
}

bool Database::testConnection() {
    try {
        pqxx::work txn(*conn);
        txn.exec("SELECT 1");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Test connection failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Device> Database::getAllDevices() {
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
    } catch (const std::exception& e) {
        std::cerr << "Error getting devices: " << e.what() << std::endl;
    }
    return devices;
}

bool Database::addDevice(const Device& device) {
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
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding device: " << e.what() << std::endl;
        return false;
    }
}

// Реализация недостающих методов для Device
bool Database::updateDevice(int id, const Device& device) {
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
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error updating device: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteDevice(int id) {
    try {
        pqxx::work txn(*conn);
        txn.exec_params("DELETE FROM Devices WHERE device_id=$1", id);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting device: " << e.what() << std::endl;
        return false;
    }
}

std::vector<ServiceType> Database::getAllServiceTypes() {
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
    } catch (const std::exception& e) {
        std::cerr << "Error getting service types: " << e.what() << std::endl;
    }
    return types;
}

// Реализация недостающих методов для ServiceType
bool Database::addServiceType(const ServiceType& type) {
    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO Service_Types (name, recommended_interval_months, standard_cost) VALUES ($1, $2, $3)",
            type.name,
            type.recommended_interval_months,
            type.standard_cost
        );
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding service type: " << e.what() << std::endl;
        return false;
    }
}

bool Database::updateServiceType(int id, const ServiceType& type) {
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
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error updating service type: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteServiceType(int id) {
    try {
        pqxx::work txn(*conn);
        txn.exec_params("DELETE FROM Service_Types WHERE service_id=$1", id);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting service type: " << e.what() << std::endl;
        return false;
    }
}

std::vector<ServiceRecord> Database::getAllServiceRecords() {
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
    } catch (const std::exception& e) {
        std::cerr << "Error getting service records: " << e.what() << std::endl;
    }
    return records;
}

bool Database::addServiceRecord(const ServiceRecord& record) {
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
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding service record: " << e.what() << std::endl;
        return false;
    }
}

// Реализация недостающих методов для ServiceRecord
bool Database::updateServiceRecord(int id, const ServiceRecord& record) {
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
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error updating service record: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteServiceRecord(int id) {
    try {
        pqxx::work txn(*conn);
        txn.exec_params("DELETE FROM Service_History WHERE record_id=$1", id);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting service record: " << e.what() << std::endl;
        return false;
    }
}

json Database::getDetailedServiceHistory() {
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
    } catch (const std::exception& e) {
        std::cerr << "Error getting detailed history: " << e.what() << std::endl;
    }
    return result;
}
