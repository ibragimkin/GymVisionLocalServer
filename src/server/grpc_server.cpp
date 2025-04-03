#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/alarm.h>
#include <grpcpp/support/async_stream.h>
#include "logger/logger.hpp"
#include "database/idatabase.hpp"
#include "GymVisionLocal.grpc.pb.h"
#include "server/handlers/start_stream.hpp"
#include "server/handlers/start_stream_cv.hpp"
class ServerImpl final {
public:
    ~ServerImpl() {
        server_->Shutdown();
        cq_->Shutdown();
    }

    ServerImpl(IDatabase* database, Logger* logger) : database_(database), logger_(logger)  {
    }

    void Run(const std::string& global_ip, const std::string& port) {
        const std::string address(global_ip + ":" + port);
        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server started. Listening on " + address<< "\n";
        logger_->LogInfo("Server started. Listening on " + address);
        HandleRpcs();
    }

private:
    void HandleRpcs() {
        new StartStreamCallData(&service_, cq_.get(), database_, logger_);
        new StartStreamCVCallData(&service_, cq_.get(), database_, logger_);

        void* tag;
        bool ok;
        while (cq_->Next(&tag, &ok)) {
            if (ok) {
                static_cast<ICallData*>(tag)->Proceed();
            } else {
                delete static_cast<ICallData*>(tag);
            }
        }
    }
    IDatabase* database_;
    Logger* logger_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    gymvision::LocalServer::AsyncService service_;
    std::unique_ptr<grpc::Server> server_;
};