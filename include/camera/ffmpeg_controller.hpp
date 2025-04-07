#pragma once
#include <string>
#include <thread>
#include <atomic>
class FfmpegController {
public:
    FfmpegController();

    bool StartStream(const std::string &ffmpeg_command);

private:
    void StreamThreadFunction(const std::string ffmpeg_command);

    // Поток, в котором запускается ffmpeg процесс
    std::thread stream_thread_;

    std::atomic<bool> running_;
};
