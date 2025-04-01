#include "config/config.cpp"
#include "server/grpc_server.cpp"
#include "database/db.cpp"
int main() {
    auto app_config = AppConfig::Load("../config/config.yaml");
    Logger logger = Logger(app_config.log_path, LogLevel::Error);
    auto database = MyDatabase()
    ServerImpl grpc_server = ServerImpl(logger, );
    grpc_server.Run(app_config.global_server_ip, app_config.grpc_port);
    return 0;
}