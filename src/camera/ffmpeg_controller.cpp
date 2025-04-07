#include "camera/ffmpeg_controller.hpp"
#include <cstdlib>    // для std::system
#include <iostream>
#include <sstream>

FfmpegController::FfmpegController()
    : running_(false)
{
}

bool FfmpegController::StartStream(const std::string &ffmpeg_command) {
    if (running_) {
        std::cerr << "Stream already running." << std::endl;
        return false;
    }
    running_ = true;
    stream_thread_ = std::thread(&FfmpegController::StreamThreadFunction, this, ffmpeg_command);
    return true;
}


void FfmpegController::StreamThreadFunction(const std::string ffmpeg_command) {
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
