#pragma once
#include <string>
#include <mutex>
#include <fstream>

enum class LogLevel {
    Info,
    Error
};

class Logger {
public:
    Logger(const std::string &log_file, LogLevel level);
    ~Logger();

    void LogInfo(const std::string &message);
    void LogError(const std::string &message);

private:
    void Log(const std::string &message, LogLevel level);
    std::string LevelToString(LogLevel level);
    std::string GetCurrentTime();

    std::string log_file_;
    LogLevel log_level_;
    std::mutex mutex_;
    std::ofstream log_stream_;
};