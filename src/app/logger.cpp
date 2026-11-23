#include "logger.h"

#include <cstdio>
#include <iostream>
#include <sstream>

Logger::Logger(const std::string& filePath) {
    if (!filePath.empty()) {
        logFile_.open(filePath, std::ios::out | std::ios::app);
    }
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.flush();
        logFile_.close();
    }
}

std::string Logger::CurrentTimestamp() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02u:%02u:%02u.%03u",
                  st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return std::string(buf);
}

std::string Logger::FormatWinError(DWORD errorCode) {
    if (errorCode == 0) {
        return "No error";
    }
    LPSTR msgBuffer = nullptr;
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD size = FormatMessageA(
        flags,
        nullptr,
        errorCode,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        reinterpret_cast<LPSTR>(&msgBuffer),
        0,
        nullptr
    );

    if (size == 0 || msgBuffer == nullptr) {
        size = FormatMessageA(
            flags,
            nullptr,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&msgBuffer),
            0,
            nullptr
        );
    }

    std::string message = "Unknown system error";
    if (size > 0 && msgBuffer != nullptr) {
        message.assign(msgBuffer, size);
        while (!message.empty() && (message.back() == '\r' || message.back() == '\n' || message.back() == ' ')) {
            message.pop_back();
        }
        LocalFree(msgBuffer);
    }
    return message;
}

void Logger::Log(const std::string& operation,
                 const std::string& object,
                 const std::string& status,
                 DWORD errorCode,
                 const std::string& details) {
    std::ostringstream oss;
    oss << "[" << CurrentTimestamp() << "] " << operation;
    if (!object.empty()) {
        oss << " target=" << object;
    }
    oss << " status=" << status;
    if (errorCode != 0) {
        oss << " code=" << errorCode;
    }
    if (!details.empty()) {
        oss << " details=\"" << details << "\"";
    }

    const std::string line = oss.str();
    std::cout << line << "\n";
    if (logFile_.is_open()) {
        logFile_ << line << "\n";
    }
}
