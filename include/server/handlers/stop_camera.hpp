#pragma once
#include "ICallData.hpp"
#include "camera/http_controller.hpp"

class StopMoveCallData : public ICallData {
public:
    StopMoveCallData(gymvision::LocalServer::AsyncService *service, grpc::ServerCompletionQueue *cq, IDatabase *db,
                     Logger *logger)
        : service_(service), cq_(cq), responder_(&ctx_), database_(db), logger_(logger), status_(CREATE) {
        Proceed();
    }

    void Proceed() override {
        if (status_ == CREATE) {
            status_ = PROCESS;
            service_->RequestStopMove(&ctx_, &request_, &responder_, cq_, cq_, this);
        } else if (status_ == PROCESS) {
            new StopMoveCallData(service_, cq_, database_, logger_); // следующий запрос
            logger_->LogInfo("New request: StopMove for camera_id: " + std::to_string(request_.camera_id()) + ".\n");
            try {
                if (database_->CameraExists(request_.camera_id())) {
                    const Camera camera = database_->GetCamera(request_.camera_id());
                    auto ptz_controller = HttpController();
                    ptz_controller.RegisterCamera(camera);
                    ptz_controller.StopCamera(camera.id);
                    logger_->LogInfo("Movement stopped for camera " + std::to_string(request_.camera_id()));
                    reply_.set_msg("DONE");
                } else {
                    logger_->LogError("Camera with id " + std::to_string(request_.camera_id()) + " is not found.\n");
                    reply_.set_msg("Camera not found.");
                }
            } catch (const std::exception &ex) {
                logger_->LogError(ex.what());
            }
            status_ = FINISH;
            responder_.Finish(reply_, grpc::Status::OK, this);
        } else {
            delete this;
        }
    }

private:
    IDatabase *database_;
    Logger *logger_;
    gymvision::LocalServer::AsyncService *service_;
    grpc::ServerCompletionQueue *cq_;
    grpc::ServerContext ctx_;
    gymvision::CameraInfo request_;
    gymvision::Response reply_;
    grpc::ServerAsyncResponseWriter<gymvision::Response> responder_;

    enum CallStatus { CREATE, PROCESS, FINISH };

    CallStatus status_;
};
