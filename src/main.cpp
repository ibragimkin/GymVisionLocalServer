#include <camera/port_manager.hpp>
#include "config/config.hpp"
#include "server/grpc_server.hpp"
#include "database/sqlitedb.hpp"
#include "camera/http_controller.hpp"
#include "camera/ffmpeg_controller.hpp"
int main() {
    auto app_config = AppConfig::Load("../config/config.yaml");
    PortManager::Init(app_config.stream_port_range.first, app_config.stream_port_range.second, app_config.http_port);
    auto logger = Logger(app_config.log_path, LogLevel::Info);
    auto database = SQLiteDB(app_config.db_path);
    auto camera = Camera("1", "TEST1", "192.168.0.37", "2020", "anisa13125", "anisanesnitch", "rtsp://anisa13125:anisanesnitch@192.168.0.37:554/stream1");
    database.AddCamera(camera);
    ServerImpl grpc_server = ServerImpl(&database, &logger);
    grpc_server.Run(app_config.global_server_ip, app_config.grpc_port);
    return 0;
}