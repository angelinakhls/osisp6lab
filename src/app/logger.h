#pragma once

#include <fstream>
#include <string>
#include <windows.h>

class Logger {
public:
    explicit Logger(const std::string& filePath = "");
    ~Logger();

    void Log(const std::string& operation,
             const std::string& object,
             const std::string& status,
             DWORD errorCode = 0,
             const std::string& details = "");

    static std::string FormatWinError(DWORD errorCode);

private:
    static std::string CurrentTimestamp();
    std::ofstream logFile_;
};
