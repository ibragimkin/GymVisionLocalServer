#include "config/config.cpp"
#include "server/grpc_server.cpp"
#include "database/sqlitedb.hpp"
int main() {
    auto app_config = AppConfig::Load("../config/config.yaml");
    auto logger = new Logger(app_config.log_path, LogLevel::Info);
    std::cout << "LOGGER_ON\n";
    logger->LogInfo("STARTED");
    auto database = new SqliteDB(app_config.db_path);
    std::cout << "DB_ON\n";
    ServerImpl grpc_server = ServerImpl(database, logger);
    std::cout << "OAOAOAO_ON\n";
    grpc_server.Run(app_config.global_server_ip, app_config.grpc_port);
    return 0;
}