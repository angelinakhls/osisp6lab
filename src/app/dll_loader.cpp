#include "dll_loader.h"

#include <iostream>
#include <sstream>

MathDllLoader::MathDllLoader(Logger& logger) : logger_(logger) {}

MathDllLoader::~MathDllLoader() {
    if (module_ != nullptr) {
        Unload();
    }
}

double MathDllLoader::NowMs() {
    static LARGE_INTEGER freq = []() {
        LARGE_INTEGER f{};
        QueryPerformanceFrequency(&f);
        return f;
    }();
    LARGE_INTEGER counter{};
    QueryPerformanceCounter(&counter);
    return (static_cast<double>(counter.QuadPart) * 1000.0) / static_cast<double>(freq.QuadPart);
}

void MathDllLoader::ReportApiError(const std::string& apiName,
                                   const std::string& operation,
                                   const std::string& target,
                                   DWORD errorCode) {
    const std::string desc = Logger::FormatWinError(errorCode);
    std::cerr << "[ERROR] Windows API call failed!\n"
              << "  Function  : " << apiName << "\n"
              << "  Operation : " << operation << "\n"
              << "  Target    : " << target << "\n"
              << "  Error Code: " << errorCode << "\n"
              << "  Message   : " << desc << "\n";
    logger_.Log(operation, target, "ERROR", errorCode, apiName + ": " + desc);
}

bool MathDllLoader::Load(const std::string& dllPath, double* outLoadTimeMs) {
    if (module_ != nullptr) {
        Unload();
    }

    requestedPath_ = dllPath;
    const double t0 = NowMs();
    module_ = LoadLibraryA(dllPath.c_str());
    const double t1 = NowMs();

    if (outLoadTimeMs != nullptr) {
        *outLoadTimeMs = t1 - t0;
    }

    if (module_ == nullptr) {
        DWORD err = GetLastError();
        ReportApiError("LoadLibraryA", "DLL_LOAD", dllPath, err);
        return false;
    }

    char pathBuf[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(module_, pathBuf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        loadedFullPath_.assign(pathBuf, len);
    } else {
        loadedFullPath_ = dllPath;
    }

    logger_.Log("DLL_LOAD", loadedFullPath_, "OK");
    return true;
}

FARPROC MathDllLoader::ResolveSymbol(const std::string& funcName) {
    if (module_ == nullptr) {
        logger_.Log("DLL_GETPROC", funcName, "ERROR", ERROR_INVALID_HANDLE, "Library is not loaded");
        return nullptr;
    }

    FARPROC proc = GetProcAddress(module_, funcName.c_str());
    if (proc == nullptr) {
        DWORD err = GetLastError();
        ReportApiError("GetProcAddress", "DLL_GETPROC", funcName, err);
        return nullptr;
    }

    logger_.Log("DLL_GETPROC", funcName, "OK");
    return proc;
}

bool MathDllLoader::ResolveAllFunctions(double* outResolveTimeMs) {
    const double t0 = NowMs();

    FARPROC pSum = ResolveSymbol("ArraySum");
    FARPROC pMin = ResolveSymbol("ArrayMin");
    FARPROC pMax = ResolveSymbol("ArrayMax");
    FARPROC pAvg = ResolveSymbol("ArrayAverage");

    const double t1 = NowMs();
    if (outResolveTimeMs != nullptr) {
        *outResolveTimeMs = t1 - t0;
    }

    if (!pSum || !pMin || !pMax || !pAvg) {
        funcs_ = MathFunctions{};
        return false;
    }

    funcs_.ArraySum     = reinterpret_cast<ArrayOpFunc>(pSum);
    funcs_.ArrayMin     = reinterpret_cast<ArrayOpFunc>(pMin);
    funcs_.ArrayMax     = reinterpret_cast<ArrayOpFunc>(pMax);
    funcs_.ArrayAverage = reinterpret_cast<ArrayOpFunc>(pAvg);
    return true;
}

bool MathDllLoader::Unload(double* outUnloadTimeMs) {
    if (module_ == nullptr) {
        return true;
    }

    const std::string pathCopy = loadedFullPath_.empty() ? requestedPath_ : loadedFullPath_;
    const double t0 = NowMs();
    BOOL ok = FreeLibrary(module_);
    const double t1 = NowMs();

    if (outUnloadTimeMs != nullptr) {
        *outUnloadTimeMs = t1 - t0;
    }

    if (!ok) {
        DWORD err = GetLastError();
        ReportApiError("FreeLibrary", "DLL_UNLOAD", pathCopy, err);
        return false;
    }

    module_ = nullptr;
    funcs_ = MathFunctions{};
    loadedFullPath_.clear();

    // Verify that the module is no longer mapped in the process address space
    HMODULE check = GetModuleHandleA(requestedPath_.c_str());
    if (check == nullptr) {
        logger_.Log("DLL_UNLOAD", pathCopy, "OK", 0, "Verified unloaded (GetModuleHandleA == NULL)");
    } else {
        logger_.Log("DLL_UNLOAD", pathCopy, "WARN", 0, "Module reference count > 0");
    }

    return true;
}
