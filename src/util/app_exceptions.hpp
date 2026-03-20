#pragma once

#include <stdexcept>
#include <string>

class AppException : public std::runtime_error {
public:
    explicit AppException(const std::string &message)
        : std::runtime_error(message) {}
};

class ValidationException : public AppException {
public:
    explicit ValidationException(const std::string &message)
        : AppException(message) {}
};

class NotFoundException : public AppException {
public:
    explicit NotFoundException(const std::string &message)
        : AppException(message) {}
};

class DatabaseException : public AppException {
public:
    explicit DatabaseException(const std::string &message)
        : AppException(message) {}
};

class BadRequestException : public AppException {
public:
    explicit BadRequestException(const std::string &message)
        : AppException(message) {}
};