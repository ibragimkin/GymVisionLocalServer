#pragma once
#include <string>
#include <thread>
#include <atomic>
#include "camera/camera.hpp"

class FfmpegController {
public:
    FfmpegController();
    std::string StartStreamCV(const Camera& camera);
    std::string StartStream(const Camera& camera);

private:
    /// 1-ая строка - listener. 2-ая - caller.
    std::pair<std::string, std::string> GenerateSrtUrls(const std::string& cam_id);
    void StreamThreadFunction(const std::string& ffmpeg_command);
    std::string GenerateSrtPassphrase(std::size_t length = 16);
    // Поток, в котором запускается ffmpeg процесс
    std::thread stream_thread_;

    std::atomic<bool> running_;
};
