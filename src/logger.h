#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <chrono>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static Logger& getInstanceImpl() {
        static Logger instance;
        return instance;
    }
    
    static std::mutex& getMutex() {
        static std::mutex mtx;
        return mtx;
    }
    
    std::ofstream logFile;
    bool logToFile;
    bool logToConsole;
    LogLevel minLevel;
    
    Logger() : logToFile(false), logToConsole(true), minLevel(LogLevel::INFO) {}
    
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
    
    // Запрещаем копирование
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
public:
    static Logger& getInstance() {
        return getInstanceImpl();
    }
    
    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(getMutex());
        logFile.open(filename, std::ios::app);
        if (logFile.is_open()) {
            logToFile = true;
        } else {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }
    
    void setLogToConsole(bool enable) {
        std::lock_guard<std::mutex> lock(getMutex());
        logToConsole = enable;
    }
    
    void setMinLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(getMutex());
        minLevel = level;
    }
    
    void log(LogLevel level, const std::string& message, const std::string& file = "", int line = -1) {
        if (level < minLevel) return;
        
        std::lock_guard<std::mutex> lock(getMutex());
        std::stringstream logEntry;
        logEntry << "[" << getCurrentTime() << "]"
                 << "[" << levelToString(level) << "]";
        
        if (!file.empty()) {
            logEntry << "[" << file;
            if (line > 0) logEntry << ":" << line;
            logEntry << "]";
        }
        
        logEntry << " " << message;
        
        if (logToConsole) {
            if (level == LogLevel::ERROR) {
                std::cerr << logEntry.str() << std::endl;
            } else {
                std::cout << logEntry.str() << std::endl;
            }
        }
        
        if (logToFile && logFile.is_open()) {
            logFile << logEntry.str() << std::endl;
            logFile.flush();
        }
    }
};

// Макросы для удобства
#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg, __FILE__, __LINE__)
