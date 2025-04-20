// http_controller.hpp
#pragma once
#include <string>
#include "camera.hpp"
class HttpController {
public:
    HttpController(const std::string& serverHost = "localhost", int serverPort = 8000);

    // Сразу шлёт синхронно
    void RegisterCamera(const Camera& camera);
    void StartStreamCV(const std::string &rtsp_url, const std::string &srt_url);
    // Асинхронно шлёт команды
    void MoveCamera(const std::string& camId, float x, float y);
    void StopCamera(const std::string& camId);

private:
    std::string _baseUrl;
};
