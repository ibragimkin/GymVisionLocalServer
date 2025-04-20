// sqlitedb.hpp
#pragma once

#include <string>
#include <stdexcept>
#include "camera/camera.hpp"
#include "idatabase.hpp"
#include <sqlite3.h>

class SQLiteDB : public IDatabase {
public:
    explicit SQLiteDB(const std::string &db_path);
    ~SQLiteDB() override;

    bool CameraExists(int camera_id) override;
    Camera GetCamera(int camera_id) override;
    void AddCamera(Camera camera) override;

private:
    bool CreateTableIfNotExists();

    sqlite3 *db_ = nullptr;
};
