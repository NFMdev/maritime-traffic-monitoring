#include "util/time_utils.hpp"

#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace time_utils {

    namespace {
        std::tm parse_tm(const std::string& value, const char *format, const char *error_context)
        {
            std::tm tm{};
            std::istringstream iss(value);
            iss >> std::get_time(&tm, format);

            if (iss.fail())
            {
                throw std::runtime_error(std::string("Failed to parse ") + error_context + ": " + value);
            }

            return tm;
        }

        std::time_t tm_as_utc_time_t(std::tm &tm)
        {
#if defined(_WIN32)
            return _mkgmtime(&tm);
#else
            return timegm(&tm);
#endif
        }

    } // namespace

    std::string to_pg_timestamp(const std::chrono::system_clock::time_point& tp)
    {
        const std::time_t time = std::chrono::system_clock::to_time_t(tp);

        std::tm utc{};
#if defined(_WIN32)
        gmtime_s(&utc, &time);
#else
        gmtime_r(&time, &utc);
#endif

        std::ostringstream oss;
        oss << std::put_time(&utc, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::chrono::system_clock::time_point from_pg_timestamp(const std::string& value) {
        std::tm tm = parse_tm(value, "%Y-%m-%d %H:%M:%S", "PostgreSQL timestamp");

        const std::time_t time = tm_as_utc_time_t(tm);
        if (time == static_cast<std::time_t>(-1))
        {
            throw std::runtime_error("Failed to convert PostgreSQL timestamp to time_t: " + value);
        }

        return std::chrono::system_clock::from_time_t(time);
    }

    std::chrono::system_clock::time_point from_iso_utc(const std::string& value) {
        std::tm tm = parse_tm(value, "%Y-%m-%dT%H:%M:%SZ", "ISO-8601 UTC timestamp");

        const std::time_t time = tm_as_utc_time_t(tm);
        if (time == static_cast<std::time_t>(-1))
        {
            throw std::runtime_error("Failed to convert ISO-8601 timestamp to time_t: " + value);
        }

        return std::chrono::system_clock::from_time_t(time);
    }

    std::string to_iso_utc(const std::chrono::system_clock::time_point& tp) {
        const std::time_t time = std::chrono::system_clock::to_time_t(tp);

        std::tm utc{};
    #if defined(_WIN32)
        gmtime_s(&utc, &time);
    #else
        gmtime_r(&time, &utc);
    #endif

        std::ostringstream oss;
        oss << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }
}
