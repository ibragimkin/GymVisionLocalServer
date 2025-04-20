// port_manager.hpp
#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include <mutex>

/// Управление пулом портов [min_port, max_port] для камер по их id
/// и получение внешнего IPv4-адреса.
class PortManager {
public:
    /// Инициализирует диапазон портов [min_port, max_port].
    static void Init(int min_port, int max_port, int http_port);

    /// Вернёт порт, выделенный за камерой id.
    /// Если ещё не было выделено — выдаст свободный из пула.
    /// \throws std::runtime_error если свободных портов нет.
    static int GetFreePort(int id);

    /// Вернёт порт, выделенный под локальный http-сервер.
    static int GetHttpPort();

    /// Освободит порт, связанный с камерой id, и вернёт его в пул.
    static void ReleasePort(int id);

    /// Вернёт ваш внешний IPv4-адрес, запрашивая его у сервиса api.ipify.org.
    /// В случае неудачи вернёт пустую строку.
    static std::string GetIPv4();

private:
    inline static int _http_port = 8000;
    inline static int _min_port    = 0;
    inline static int _max_port    = -1;
    inline static std::set<int> _free_ports;
    inline static std::unordered_map<int,int> _allocated;
    inline static std::mutex _mtx;
    inline static std::string _cached_ip;
};
