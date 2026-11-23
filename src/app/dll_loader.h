#pragma once

#include "logger.h"

#include <cstddef>
#include <string>
#include <windows.h>

using ArrayOpFunc = bool(__cdecl*)(const double*, std::size_t, double*);

struct MathFunctions {
    ArrayOpFunc ArraySum = nullptr;
    ArrayOpFunc ArrayMin = nullptr;
    ArrayOpFunc ArrayMax = nullptr;
    ArrayOpFunc ArrayAverage = nullptr;
};

class MathDllLoader {
public:
    explicit MathDllLoader(Logger& logger);
    ~MathDllLoader();

    MathDllLoader(const MathDllLoader&) = delete;
    MathDllLoader& operator=(const MathDllLoader&) = delete;

    bool Load(const std::string& dllPath, double* outLoadTimeMs = nullptr);
    bool ResolveAllFunctions(double* outResolveTimeMs = nullptr);
    FARPROC ResolveSymbol(const std::string& funcName);
    bool Unload(double* outUnloadTimeMs = nullptr);

    bool IsLoaded() const { return module_ != nullptr; }
    const std::string& LoadedPath() const { return loadedFullPath_; }
    const MathFunctions& Functions() const { return funcs_; }

    static double NowMs();

private:
    void ReportApiError(const std::string& apiName,
                        const std::string& operation,
                        const std::string& target,
                        DWORD errorCode);

    Logger& logger_;
    HMODULE module_ = nullptr;
    std::string requestedPath_;
    std::string loadedFullPath_;
    MathFunctions funcs_{};
};
