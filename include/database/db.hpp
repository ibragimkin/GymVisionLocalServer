#pragma once
#include <string>

class IDatabase {
public:
    virtual bool CameraExists(int camera_id) = 0;
    virtual void LogEvent(const std::string& type, int camera_id, const std::string& msg) = 0;
    virtual ~IDatabase() = default;
};
