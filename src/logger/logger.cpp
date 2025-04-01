#include "logger/logger.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iostream>

Logger::Logger(const std::string &log_file, LogLevel level)
    : log_file_(log_file), log_level_(level) {
    log_stream_.open(log_file_, std::ios::app);
    if (!log_stream_.is_open()) {
        std::cerr << "Failed to open log file: " << log_file_ << std::endl;
    }
}

Logger::~Logger() {
    if (log_stream_.is_open()) {
        log_stream_.close();
    }
}

void Logger::LogInfo(const std::string &message) {
    if (log_level_ <= LogLevel::Info) {
        Log(message, LogLevel::Info);
    }
}

void Logger::LogError(const std::string &message) {
    if (log_level_ <= LogLevel::Error) {
        Log(message, LogLevel::Error);
    }
}

void Logger::Log(const std::string &message, LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_stream_.is_open()) {
        log_stream_ << "[" << GetCurrentTime() << "] " << LevelToString(level) << ": " << message << "\n";
        log_stream_.flush();
    } else {
        std::cerr << "[" << GetCurrentTime() << "] " << LevelToString(level) << ": " << message << "\n";
    }
}

std::string Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Info:  return "INFO";
        case LogLevel::Error: return "ERROR";
        default:              return "UNKNOWN";
    }
}

std::string Logger::GetCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::string time_str = std::ctime(&now_time);
    if (!time_str.empty() && time_str.back() == '\n') {
        time_str.pop_back();
    }
    return time_str;
}
