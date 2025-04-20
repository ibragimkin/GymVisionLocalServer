// port_manager.cpp
#include "camera/port_manager.hpp"
#include <stdexcept>
#include <cstdio>
#include <array>

void PortManager::Init(int min_port, int max_port, int http_port) {
    std::lock_guard lock(_mtx);
    if (min_port > max_port) {
        throw std::invalid_argument("PortManager::Init: min_port > max_port");
    }
    _min_port = min_port;
    _max_port = max_port;
    _http_port = http_port;
    _allocated.clear();
    _free_ports.clear();
    for (int p = _min_port; p <= _max_port; ++p) {
        _free_ports.insert(p);
    }
}

int PortManager::GetFreePort(int id) {
    std::lock_guard lock(_mtx);
    auto it = _allocated.find(id);
    if (it != _allocated.end()) {
        return it->second;
    }
    if (_free_ports.empty()) {
        throw std::runtime_error("PortManager::GetFreePort: no free ports");
    }
    int port = *_free_ports.begin();
    _free_ports.erase(_free_ports.begin());
    _allocated[id] = port;
    return port;
}

void PortManager::ReleasePort(int id) {
    std::lock_guard lock(_mtx);
    auto it = _allocated.find(id);
    if (it == _allocated.end()) {
        return;
    }
    _free_ports.insert(it->second);
    _allocated.erase(it);
}

std::string PortManager::GetIPv4() {
    if (!_cached_ip.empty()) {
        return _cached_ip;
    }
    // Используем внешний сервис api.ipify.org для определения IP
    const char* cmd = "curl -s https://api.ipify.org";
    std::array<char, 128> buffer{};
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        return "";
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        result += buffer.data();
    }
    pclose(pipe);
    // Убираем возможный '\n' в конце
    if (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    _cached_ip = result;
    return result;
}

int PortManager::GetHttpPort() {
    return _http_port;
}
