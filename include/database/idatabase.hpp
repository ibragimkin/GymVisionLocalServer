#pragma once
#include <string>
#include "camera/camera.hpp"
class IDatabase {
public:
    virtual bool CameraExists(int camera_id) = 0;
    virtual Camera GetCamera(int camera_id) = 0;
    virtual void AddCamera(Camera camera) = 0;
    virtual ~IDatabase() = default;
};
