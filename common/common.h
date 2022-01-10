/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 * 
 */

#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace common {
    const char FIELD_SEP = ' ';

    std::string trim(const std::string& str);

    typedef std::chrono::high_resolution_clock clock_t;
    typedef std::unordered_map<std::string, std::function<void()>> CallbackMap;

    std::chrono::milliseconds get_current_milliseconds();

    std::pair<std::string, int> parse_ip_port(const std::string& arg);
    std::string collapse_line(const std::string& message);

} // namespace common

