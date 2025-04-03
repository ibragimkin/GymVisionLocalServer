#include "database/sqlitedb.hpp"
#include <iostream>

SqliteDB::SqliteDB(const std::string &db_path)
    : db_(nullptr)
{
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
    }

    // Если база открыта – создаём таблицу (если её ещё нет)
    if (db_ && !CreateTableIfNotExists()) {
        std::cerr << "Failed to create table in database." << std::endl;
    }
}

SqliteDB::~SqliteDB() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool SqliteDB::CreateTableIfNotExists() {
    // SQL-запрос для создания таблицы cameras с нужными полями
    const char *sql = "CREATE TABLE IF NOT EXISTS cameras ("
                      "id INTEGER PRIMARY KEY, "
                      "name TEXT NOT NULL, "
                      "ip TEXT NOT NULL, "
                      "video_ip TEXT NOT NULL);";
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creating table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool SqliteDB::CameraExists(int camera_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    const char *sql = "SELECT COUNT(*) FROM cameras WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement in CameraExists: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, camera_id);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        exists = (count > 0);
    }
    sqlite3_finalize(stmt);
    return exists;
}

std::string SqliteDB::GetCameraIP(int camera_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    const char *sql = "SELECT ip FROM cameras WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    std::string ip;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement in GetCameraIP: " << sqlite3_errmsg(db_) << std::endl;
        return "";
    }
    sqlite3_bind_int(stmt, 1, camera_id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        ip = text ? reinterpret_cast<const char*>(text) : "";
    }
    sqlite3_finalize(stmt);
    return ip;
}

std::string SqliteDB::GetCameraVideoIP(int camera_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    const char *sql = "SELECT video_ip FROM cameras WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    std::string video_ip;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement in GetCameraVideoIP: " << sqlite3_errmsg(db_) << std::endl;
        return "";
    }
    sqlite3_bind_int(stmt, 1, camera_id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        video_ip = text ? reinterpret_cast<const char*>(text) : "";
    }
    sqlite3_finalize(stmt);
    return video_ip;
}

void SqliteDB::AddCamera(int camera_id, std::string name, std::string ip, std::string video_ip) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    if (CameraExists(camera_id)) {
        std::cerr << "Camera with id " << camera_id << " already exists." << std::endl;
        return;
    }

    const char *sql = "INSERT INTO cameras (id, name, ip, video_ip) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement in AddCamera: " << sqlite3_errmsg(db_) << std::endl;
        return;
    }
    sqlite3_bind_int(stmt, 1, camera_id);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ip.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, video_ip.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Error inserting camera: " << sqlite3_errmsg(db_) << std::endl;
    }
    sqlite3_finalize(stmt);
}
