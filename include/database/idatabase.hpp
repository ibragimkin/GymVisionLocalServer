#pragma once
#include <string>

class IDatabase {
public:
    virtual bool CameraExists(int camera_id) = 0;
    virtual std::string GetCameraIP(int camera_id) = 0;
    virtual std::string GetCameraVideoIP(int camera_id) = 0;
    virtual void AddCamera(int camera_id, std::string name, std::string ip, std::string video_ip) = 0;
    virtual ~IDatabase() = default;
};
