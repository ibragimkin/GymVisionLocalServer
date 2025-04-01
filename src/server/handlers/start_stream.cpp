class StartStreamCallData : public ICallData {
public:
    StartStreamCallData(gymvision::LocalServer::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
        Proceed();
    }

    void Proceed() override {
        if (status_ == CREATE) {
            status_ = PROCESS;
            service_->RequestStartStream(&ctx_, &request_, &responder_, cq_, cq_, this);
        } else if (status_ == PROCESS) {
            new StartStreamCallData(service_, cq_); // следующий запрос

            reply_.set_message("Hello, " + request_.name());
            status_ = FINISH;
            responder_.Finish(reply_, grpc::Status::OK, this);
        } else {
            delete this;
        }
    }

private:
    gymvision::LocalServer::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    gymvision::CameraInfo request_;
    gymvision::StreamInfo reply_;
    grpc::ServerAsyncResponseWriter<gymvision::StreamInfo> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
};
