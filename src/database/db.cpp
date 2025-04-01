#include "database/db.hpp"  // Предполагается, что в db.hpp объявлен интерфейс IDatabase
#include <sqlite3.h>
#include <iostream>
#include <mutex>
#include <string>

// Реализация класса MyDatabase, наследующего IDatabase
class MyDatabase : public IDatabase {
public:
    // Конструктор, принимает путь к файлу базы данных
    MyDatabase(const std::string &db_path);

    // Деструктор, закрывающий соединение с БД
    virtual ~MyDatabase();

    // Проверяет, существует ли запись о камере с заданным id
    bool CameraExists(int camera_id) override;

private:
    sqlite3 *db_;              // Указатель на объект SQLite базы данных
    std::mutex db_mutex_;      // Мьютекс для синхронизации доступа к БД
};

MyDatabase::MyDatabase(const std::string &db_path) : db_(nullptr) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

MyDatabase::~MyDatabase() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool MyDatabase::CameraExists(int camera_id) {
    if (!db_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(db_mutex_);

    const char *query = "SELECT COUNT(*) FROM cameras WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Привязываем параметр camera_id
    sqlite3_bind_int(stmt, 1, camera_id);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return count > 0;
}
