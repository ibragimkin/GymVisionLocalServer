#include <string>
#include <yaml-cpp/yaml.h>

struct AppConfig {
    std::string global_server_ip;
    std::string grpc_port;
    std::string http_port;
    std::string db_path;
    std::string log_path;
    std::string log_level;
    std::pair<int, int> stream_port_range;

    static AppConfig Load(const std::string& path) {
        YAML::Node config = YAML::LoadFile(path);
        AppConfig cfg;
        cfg.global_server_ip = config["server"]["global_server_ip"].as<std::string>();
        cfg.grpc_port = config["server"]["grpc_port"].as<std::string>();
        cfg.http_port = config["server"]["http_port"].as<std::string>();

        cfg.db_path = config["database"]["path"].as<std::string>();

        cfg.log_path = config["log"]["path"].as<std::string>();
        cfg.log_level = config["log"]["level"].as<std::string>();

        cfg.stream_port_range.first = config["ffmpeg"]["stream_port_range"]["min"].as<int>();
        cfg.stream_port_range.second = config["ffmpeg"]["stream_port_range"]["min"].as<int>();

        return cfg;
    }
};
