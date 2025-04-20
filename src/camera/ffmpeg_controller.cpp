#include "camera/ffmpeg_controller.hpp"
#include "camera/http_controller.hpp"
#include "camera/port_manager.hpp"
#include <cstdlib>    // для std::system
#include <iostream>
#include <sstream>
#include <random>
FfmpegController::FfmpegController()
    : running_(false)
{
}

std::pair<std::string, std::string> FfmpegController::GenerateSrtUrls(const std::string& cam_id) {
    int port = PortManager::GetFreePort(std::stoi(cam_id));
    std::string passphrase = GenerateSrtPassphrase(10);
    std::string listener_url = "srt://0.0.0.0:" + std::to_string(port) + "?mode=listener&pkt_size=1316&latency=200&passphrase="+passphrase;
    std::string caller_url = "srt://localhost:" + std::to_string(port) + "?mode=caller&pkt_size=1316&latency=200&passphrase=" + passphrase;
    return {listener_url, caller_url};
}

std::string FfmpegController::StartStream(const Camera& camera) {
    if (running_) {
        std::cerr << "Stream already running." << std::endl;
        return "null";
    }
    running_ = true;
    auto urls = GenerateSrtUrls(camera.id);
    std::string ffmpeg_command = "ffmpeg -i " + camera.video_url  +" -c:v libx264 -b:v 2500k -flags low_delay -preset ultrafast -tune zerolatency -f mpegts \"" + urls.first+"\"";
    stream_thread_ = std::thread(&FfmpegController::StreamThreadFunction,
                                this, ffmpeg_command);
    stream_thread_.detach();
    return urls.second;
}

void FfmpegController::StreamThreadFunction(const std::string& ffmpeg_command) {
    std::ostringstream command_stream;
    command_stream << ffmpeg_command;
    std::string command = command_stream.str();

    std::cout << "Starting ffmpeg with command: " << command << std::endl;

    int ret = std::system(command.c_str());
    if (ret != 0) {
        std::cerr << "ffmpeg exited with code: " << ret << std::endl;
    }
    running_ = false;
}

std::string FfmpegController::StartStreamCV(const Camera& camera) {
    if (running_) {
        std::cerr << "Stream already running." << std::endl;
        return "null";
    }
    running_ = true;
    auto urls = GenerateSrtUrls(camera.id);
    HttpController controller = HttpController();
    controller.StartStreamCV(camera.video_url, urls.first);
    return urls.second;
}

/// Возвращает пароль длины length, состоящий из [0-9A-Za-z]
std::string FfmpegController::GenerateSrtPassphrase(const std::size_t length) {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937_64 rng{std::random_device{}()};
    static std::uniform_int_distribution<std::size_t> dist(
        0, sizeof(charset) - 2
    );

    std::string s;
    s.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        s += charset[dist(rng)];
    }
    return s;
}