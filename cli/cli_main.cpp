#include <iostream>
#include <camera/http_controller.hpp>
#include "config/config.hpp"
#include "CLI/CLI.hpp"
#include "database/sqlitedb.hpp"
#include "camera/ffmpeg_controller.hpp"
#include "camera/port_manager.hpp"
#include "camera/http_controller.hpp"
int main(int argc, char** argv) {
    auto app_config = AppConfig::Load("../config/config.yaml");
    PortManager::Init(app_config.stream_port_range.first, app_config.stream_port_range.second, app_config.http_port);
    auto db = SQLiteDB(app_config.db_path);
    FfmpegController ffmpeg_controller;
    HttpController http_controller = HttpController();
    CLI::App app{"Camera Streaming Server CLI"};
    // add-camera command
    int add_id;
    std::string add_name, add_ip, add_port, add_username, add_password, add_video_url;
    auto cmd_add = app.add_subcommand("add-camera", "Add a camera to the database");
    cmd_add->add_option("id", add_id, "Camera ID")->required();
    cmd_add->add_option("name", add_name, "Camera name")->required();
    cmd_add->add_option("ip", add_ip, "Camera IP address")->required();
    cmd_add->add_option("port", add_port, "Camera port")->required();
    cmd_add->add_option("username", add_username, "Camera username")->required();
    cmd_add->add_option("password", add_password, "Camera password")->required();
    cmd_add->add_option("video_url", add_video_url, "Camera video URL")->required();
    cmd_add->callback([&]() {
        std::string full_ip = add_ip + ":" + add_port;
        db.AddCamera(Camera(std::to_string(add_id), add_name, add_ip, add_port, add_username, add_password, add_video_url));
        std::cout << "Camera " << add_id << " added." << std::endl;
    });

    // start-stream command
    int camera_id;
    auto cmd_start = app.add_subcommand("start-stream", "Start streaming from a camera");
    cmd_start->add_option("id", camera_id, "Camera ID")->required();
    cmd_start->callback([&]() {
        if (!db.CameraExists(camera_id)) {
            std::cerr << "Camera " << camera_id << " does not exist." << std::endl;
            return;
        }
        auto camera = db.GetCamera(camera_id);
        std::string url = ffmpeg_controller.StartStream(camera);
        std::cout << "SRT URL: \n" <<  url;
    });

    // start-stream-cv command
    auto cmd_start_cv = app.add_subcommand("start-stream-cv", "Start streaming with computer vision overlay");
    cmd_start_cv->add_option("id", camera_id, "Camera ID")->required();
    cmd_start_cv->callback([&]() {
        if (!db.CameraExists(camera_id)) {
            std::cerr << "Camera " << camera_id << " does not exist." << std::endl;
            return;
        }
        auto camera = db.GetCamera(camera_id);
        std::cout << "SRT URL: \n" << ffmpeg_controller.StartStreamCV(camera);
    });

    // check-camera command
    auto cmd_check = app.add_subcommand("check-camera", "Check if a camera exists");
    cmd_check->add_option("id", camera_id, "Camera ID")->required();
    cmd_check->callback([&]() {
        bool exists = db.CameraExists(camera_id);
        std::cout << "Camera " << camera_id << (exists ? " exists." : " does not exist.") << std::endl;
    });

    // move-camera command
    float move_x, move_y;
    int time;
    auto cmd_move = app.add_subcommand("move-camera", "Move camera PTZ by x and y for time");
    cmd_move->add_option("id", camera_id, "Camera ID")->required();
    cmd_move->add_option("x", move_x, "Pan speed")->required();
    cmd_move->add_option("y", move_y, "Tilt speed")->required();
    cmd_move->add_option("time", time, "Time of movement (seconds)")->required();
    cmd_move->callback([&]() {
        if (!db.CameraExists(camera_id)) {
            std::cerr << "Camera " << camera_id << " does not exist." << std::endl;
            return;
        }
        auto camera = db.GetCamera(camera_id);
        http_controller.RegisterCamera(camera);
        http_controller.MoveCamera(std::to_string(camera_id), move_x, move_y);
        std::this_thread::sleep_for(std::chrono::seconds(time));
        http_controller.StopCamera(std::to_string(camera_id));
        std::cout << "Camera " << camera_id << " moved to (" << move_x << ", " << move_y << ")." << std::endl;
    });

    app.require_subcommand(1); // at least one subcommand is required
    CLI11_PARSE(app, argc, argv);

    return 0;
}
