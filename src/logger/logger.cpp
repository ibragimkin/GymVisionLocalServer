#include "logger/logger.hpp"
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>

Logger::Logger(const std::string &log_file, LogLevel level)
    : log_level_(level) {
    log_stream_.open(log_file, std::ios::app);
    if (!log_stream_.is_open()) {
        std::cerr << "Failed to open log file: " << log_file << "\n";
    }
    worker_thread_ = std::thread(&Logger::ProcessQueue, this);
}

Logger::~Logger() {
    // Сигнализируем потоку, что пора завершаться
    exit_flag_.store(true);
    queue_cv_.notify_one();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    if (log_stream_.is_open()) {
        log_stream_.close();
    }
}

void Logger::LogInfo(const std::string &message) {
    if (log_level_ <= LogLevel::Info) {
        EnqueueLog(message, LogLevel::Info);
    }
}

void Logger::LogError(const std::string &message) {
    if (log_level_ <= LogLevel::Error) {
        EnqueueLog(message, LogLevel::Error);
    }
}

void Logger::EnqueueLog(const std::string &message, LogLevel level) {
    auto entry = FormatEntry(message, level); {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_.push(std::move(entry));
    }
    queue_cv_.notify_one();
}

void Logger::ProcessQueue() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    while (!exit_flag_.load() || !queue_.empty()) {
        // Ждём либо нового элемента, либо сигнала выхода
        queue_cv_.wait(lock, [&] {
            return exit_flag_.load() || !queue_.empty();
        });

        // Обрабатываем все накопившиеся записи
        while (!queue_.empty()) {
            std::string entry = std::move(queue_.front());
            queue_.pop();
            lock.unlock(); {
                std::lock_guard<std::mutex> file_lock(file_mutex_);
                if (log_stream_.is_open()) {
                    log_stream_ << entry << "\n";
                    log_stream_.flush();
                } else {
                    std::cerr << entry << "\n";
                }
            }

            lock.lock();
        }
    }
}

std::string Logger::FormatEntry(const std::string &message, LogLevel level) {
    std::ostringstream oss;
    oss << "[" << GetCurrentTime() << "] "
            << LevelToString(level) << ": "
            << message;
    return oss.str();
}

std::string Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Info: return "INFO";
        case LogLevel::Error: return "ERROR";
        default: return "UNKNOWN";
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
