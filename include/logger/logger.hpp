#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <atomic>

enum class LogLevel { Info, Error };

class Logger {
public:
    Logger(const std::string &log_file, LogLevel level);
    ~Logger();

    void LogInfo(const std::string &message);
    void LogError(const std::string &message);

private:
    void EnqueueLog(const std::string &message, LogLevel level);
    void ProcessQueue();
    std::string FormatEntry(const std::string &message, LogLevel level);
    std::string LevelToString(LogLevel level);
    std::string GetCurrentTime();

    std::ofstream               log_stream_;
    LogLevel                    log_level_;

    // Для очереди
    std::queue<std::string>     queue_;
    std::mutex                  queue_mutex_;
    std::condition_variable     queue_cv_;
    std::atomic<bool>           exit_flag_{false};
    std::thread                 worker_thread_;

    // Для защиты записи в файл (на случай, если вы захотите делить файл между потоками)
    std::mutex                  file_mutex_;
};
