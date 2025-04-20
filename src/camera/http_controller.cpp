#include "camera/http_controller.hpp"
#include "camera/camera.hpp"
#include <cstdlib>
#include <thread>
#include <iostream>
HttpController::HttpController(const std::string &serverHost, int serverPort)
  : _baseUrl("http://" + serverHost + ":" + std::to_string(serverPort)) {}

// POST /register  (form data)
void HttpController::RegisterCamera(const Camera &camera) {
    std::string cmd =
        "curl -s -X POST " + _baseUrl + "/register "
        "-d \"cam_id=" + camera.id       + "\" "
        "-d \"host="    + camera.ip       + "\" "
        "-d \"port="    + camera.port     + "\" "
        "-d \"user="    + camera.user     + "\" "
        "-d \"pwd="     + camera.password + "\"";
    std::system(cmd.c_str());
}

// POST /move?cam_id=...&x=...&y=...
void HttpController::MoveCamera(const std::string &camId, float x, float y) {
    std::thread([=]() {
        std::string cmd = "curl -s -X POST \"http://localhost:8000/move?cam_id=" + camId + "&x=" + std::to_string(x) + "&y=" + std::to_string(y)+"\"";

        std::system(cmd.c_str());
    }).detach();
}

// POST /stop?cam_id=...
void HttpController::StopCamera(const std::string &camId) {
    std::thread([=]() {
        std::string cmd = "curl -s -X POST \"" + _baseUrl + "/stop?cam_id=" + camId + "\"";

        std::system(cmd.c_str());
    }).detach();
}

void HttpController::StartStreamCV(const std::string &rtsp_url, const std::string &srt_url) {
    std::thread([=]() {
        std::string cmd = "curl -s -X POST \"http://localhost:8000/start_stream_cv?rtsp_url=rtsp://anisa13125:anisanesnitch@192.168.0.37:554/stream1&srt_url=srt://0.0.0.0:7002?mode=listener&latency=0\"";
        std::system(cmd.c_str());
    }).detach();
}