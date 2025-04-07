#pragma once

#include <string>
#include "rapidonvif/include/onvifclient.hpp"

class PTZController {
public:
    // Упрощённый конструктор: только адрес ONVIF устройства
    PTZController(const std::string& deviceUrl, const std::string& username, const std::string& password);

    bool move(float x, float y, int duration_sec = 1) {
        OnvifClientDevice device(ptzUrl_, username_, password_);

        OnvifClientPTZ ptz(device);
        OnvifClientMedia media(device);
        _trt__GetProfilesResponse profiles_response;
        media.GetProfiles(profiles_response);
        auto profile = profiles_response.Profiles[0];
        tt__PTZSpeed speed;

        speed.PanTilt->x = x;
        speed.PanTilt->y = y;
        speed.Zoom->x = 0;
        _tptz__ContinuousMoveResponse continuous_move_response;
        LONG64 timeout = 1;
        int cont_move_result = ptz.ContinuousMove(continuous_move_response, speed, timeout, profile->token);
        // tt__PTZVector vector = tt__PTZVector();
        // vector.PanTilt->x = 0.1f;
        // vector.PanTilt->y = 0.0f;
        // _tptz__RelativeMoveResponse response;
        // int result = ptz.RelativeMove(response, vector, speed, profile->token);
    }

private:
    std::string ptzUrl_;
    std::string username_;
    std::string password_;
    std::string profileToken_;
    bool initFromDeviceUrl(const std::string& deviceUrl);  // Получение XAddr и токена
};
