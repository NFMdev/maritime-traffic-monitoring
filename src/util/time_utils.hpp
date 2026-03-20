#pragma once

#include <string>
#include <chrono>

namespace time_utils {
    std::string to_pg_timestamp(const std::chrono::system_clock::time_point& tp);
    std::chrono::system_clock::time_point from_pg_timestamp(const std::string& value);
    std::chrono::system_clock::time_point from_iso_utc(const std::string& value);
    std::string to_iso_utc(const std::chrono::system_clock::time_point& tp);
}