#pragma once

#include <string>

/// Описывает конфигурацию и состояние одной ONVIF-камеры.
class Camera {
public:
    // Конструктор для полного наполнения полей
    Camera(std::string id,
           std::string name,
           std::string ip,
           std::string port,
           std::string user,
           std::string password,
           std::string video_url)
      : id(std::move(id))
      , name(std::move(name))
      , ip(std::move(ip))
      , port(std::move(port))
      , user(std::move(user))
      , password(std::move(password))
      , video_url(std::move(video_url))
    {}
    std::string id;
    std::string name;
    std::string ip;
    std::string port;
    std::string user;
    std::string password;
    std::string video_url;
};
