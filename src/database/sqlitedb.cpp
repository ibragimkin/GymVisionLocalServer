#include "database/sqlitedb.hpp"

// sqlitedb.cpp
SQLiteDB::SQLiteDB(const std::string &db_path)
    : db_(nullptr)
{
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db_)));
    }
    if (!CreateTableIfNotExists()) {
        sqlite3_close(db_);
        throw std::runtime_error("Failed to create table: " + std::string(sqlite3_errmsg(db_)));
    }
}


SQLiteDB::~SQLiteDB() {
    if (db_) sqlite3_close(db_);
}

bool SQLiteDB::CreateTableIfNotExists() {
    static constexpr const char *sql = R"sql(
        CREATE TABLE IF NOT EXISTS cameras (
            id          INTEGER PRIMARY KEY,
            name        TEXT    NOT NULL,
            ip          TEXT    NOT NULL,
            port        TEXT    NOT NULL,
            user        TEXT    NOT NULL,
            password    TEXT    NOT NULL,
            video_url   TEXT    NOT NULL
        );
    )sql";
    char *errmsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errmsg);
        return false;
    }
    return true;
}

bool SQLiteDB::CameraExists(int camera_id) {
    static constexpr const char *sql = "SELECT 1 FROM cameras WHERE id = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int(stmt, 1, camera_id);
    int rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

Camera SQLiteDB::GetCamera(int camera_id) {
    static constexpr const char *sql =
        "SELECT id, name, ip, port, user, password, video_url "
        "FROM cameras WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare GetCamera stmt");
    }
    sqlite3_bind_int(stmt, 1, camera_id);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Camera not found: id=" + std::to_string(camera_id));
    }

    // Читаем поля
    int        id         = sqlite3_column_int(stmt, 0);
    std::string name      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string ip        = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string port      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    std::string user      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    std::string password  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    std::string video_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

    sqlite3_finalize(stmt);

    // Преобразуем id в строку для Camera
    return Camera{
        std::to_string(id),
        name,
        ip,
        port,
        user,
        password,
        video_url
    };
}

void SQLiteDB::AddCamera(Camera camera) {
    static constexpr const char *sql =
        "INSERT INTO cameras (id, name, ip, port, user, password, video_url) "
        "VALUES (?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "  name=excluded.name, ip=excluded.ip, port=excluded.port, "
        "  user=excluded.user, password=excluded.password, video_url=excluded.video_url;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare AddCamera stmt");
    }

    // Привязываем параметры
    int id_int = std::stoi(camera.id);
    sqlite3_bind_int(stmt,    1, id_int);
    sqlite3_bind_text(stmt,   2, camera.name.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   3, camera.ip.c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   4, camera.port.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   5, camera.user.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   6, camera.password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   7, camera.video_url.c_str(),-1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw std::runtime_error("Failed to execute AddCamera stmt");
    }
}
