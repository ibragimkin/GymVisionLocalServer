#include "camera/ptz_controller.hpp"
#include "soapDeviceBindingProxy.h"
#include "soapMediaBindingProxy.h"
#include "soapPTZBindingProxy.h"
#include "wsseapi.h"
#include <iostream>
#include <thread>
#include <chrono>

PTZController::PTZController(const std::string& deviceUrl, const std::string& username, const std::string& password)
    : username_(username), password_(password) {
    initFromDeviceUrl(deviceUrl);
}

bool PTZController::initFromDeviceUrl(const std::string& deviceUrl) {
    DeviceBindingProxy dev;
    soap_wsse_add_UsernameTokenDigest(dev.soap, NULL, username_.c_str(), password_.c_str());

    _tds__GetCapabilities req;
    _tds__GetCapabilitiesResponse resp;

    int res = dev.GetCapabilities(deviceUrl.c_str(), NULL, &req, &resp);
    if (res != SOAP_OK || !resp.Capabilities || !resp.Capabilities->PTZ || !resp.Capabilities->Media) {
        dev.soap_stream_fault(std::cerr);
        return false;
    }

    ptzUrl_ = resp.Capabilities->PTZ->XAddr;

    // Получаем profileToken через MediaService
    MediaBindingProxy media;
    soap_wsse_add_UsernameTokenDigest(media.soap, NULL, username_.c_str(), password_.c_str());

    _trt__GetProfiles profReq;
    _trt__GetProfilesResponse profResp;
    res = media.GetProfiles(resp.Capabilities->Media->XAddr.c_str(), NULL, &profReq, &profResp);
    if (res != SOAP_OK || profResp.Profiles.empty()) {
        media.soap_stream_fault(std::cerr);
        return false;
    }

    profileToken_ = profResp.Profiles[0]->token;
    return true;
}

bool PTZController::move(float x, float y, float duration_sec) {
    if (ptzUrl_.empty() || profileToken_.empty()) return false;

    PTZBindingProxy ptz;
    soap_wsse_add_UsernameTokenDigest(ptz.soap, NULL, username_.c_str(), password_.c_str());

    _tptz__ContinuousMove moveRequest;
    _tptz__ContinuousMoveResponse moveResponse;
    moveRequest.ProfileToken = profileToken_;
    moveRequest.Velocity = new tt__PTZSpeed();
    moveRequest.Velocity->PanTilt = new tt__Vector2D();
    moveRequest.Velocity->PanTilt->x = x;
    moveRequest.Velocity->PanTilt->y = y;

    int result = ptz.ContinuousMove(ptzUrl_.c_str(), NULL, &moveRequest, &moveResponse);
    if (result != SOAP_OK) {
        ptz.soap_stream_fault(std::cerr);
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(int(duration_sec * 1000)));

    _tptz__Stop stopRequest;
    _tptz__StopResponse stopResponse;
    stopRequest.ProfileToken = profileToken_;
    stopRequest.PanTilt = true;

    result = ptz.Stop(ptzUrl_.c_str(), NULL, &stopRequest, &stopResponse);
    if (result != SOAP_OK) {
        ptz.soap_stream_fault(std::cerr);
        return false;
    }

    return true;
}
