#pragma once
#include "idatabase.hpp"
#include <sqlite3.h>
#include <mutex>
#include <string>

class SqliteDB : public IDatabase {
public:
    // Конструктор принимает путь к базе данных
    explicit SqliteDB(const std::string &db_path);
    virtual ~SqliteDB();

    // Проверяет, существует ли камера с заданным ID
    bool CameraExists(int camera_id) override;

    // Возвращает ip камеры по её ID
    std::string GetCameraIP(int camera_id) override;

    // Возвращает video_ip камеры по её ID
    std::string GetCameraVideoIP(int camera_id) override;

    // Добавляет новую камеру с полями: id, name, ip, video_ip
    void AddCamera(int camera_id, std::string name, std::string ip, std::string video_ip) override;

private:
    // Создаёт таблицу, если она не существует
    bool CreateTableIfNotExists();

    sqlite3 *db_;
    std::mutex db_mutex_;
};
