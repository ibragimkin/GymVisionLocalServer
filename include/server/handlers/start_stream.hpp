#pragma once
#include "ICallData.hpp"
#include "camera/ffmpeg_controller.hpp"
class StartStreamCallData : public ICallData {
public:
    StartStreamCallData(gymvision::LocalServer::AsyncService* service, grpc::ServerCompletionQueue* cq, IDatabase* db, Logger* logger)
        : service_(service), cq_(cq), responder_(&ctx_), database_(db), logger_(logger), status_(CREATE) {
        Proceed();
    }

    void Proceed() override {
        if (status_ == CREATE) {
            status_ = PROCESS;
            service_->RequestStartStream(&ctx_, &request_, &responder_, cq_, cq_, this);
        } else if (status_ == PROCESS) {
            new StartStreamCallData(service_, cq_, database_, logger_); // следующий запрос
            std::thread t(&Logger::LogInfo, logger_, "New request: StartStream for camera_id: " + std::to_string(request_.camera_id()) + ".\n");
            logger_->LogInfo("New request: StartStream for camera_id: " + std::to_string(request_.camera_id()) + ".\n");
            if (database_->CameraExists(request_.camera_id())) {
                // TODO
                // TODO
                reply_.set_stream_url(""); // DOPISAT
            } else {
                logger_->LogError("Camera with id " + std::to_string(request_.camera_id()) + " is not found.\n");
                reply_.set_stream_url("null");
            }
            status_ = FINISH;
            responder_.Finish(reply_, grpc::Status::OK, this);
        } else {
            delete this;
        }
    }

private:
    IDatabase* database_;
    Logger* logger_;
    gymvision::LocalServer::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    gymvision::CameraInfo request_;
    gymvision::StreamInfo reply_;
    grpc::ServerAsyncResponseWriter<gymvision::StreamInfo> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
};
