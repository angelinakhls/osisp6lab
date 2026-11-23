#include "math_dll.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    (void)hModule;
    (void)lpReserved;
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

extern "C" {

MATH_DLL_API bool MATH_CALL ArraySum(const double* data, std::size_t count, double* outResult) {
    if (data == nullptr || outResult == nullptr || count == 0) {
        return false;
    }
    double sum = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        sum += data[i];
    }
    *outResult = sum;
    return true;
}

MATH_DLL_API bool MATH_CALL ArrayMin(const double* data, std::size_t count, double* outResult) {
    if (data == nullptr || outResult == nullptr || count == 0) {
        return false;
    }
    double minVal = data[0];
    for (std::size_t i = 1; i < count; ++i) {
        if (data[i] < minVal) {
            minVal = data[i];
        }
    }
    *outResult = minVal;
    return true;
}

MATH_DLL_API bool MATH_CALL ArrayMax(const double* data, std::size_t count, double* outResult) {
    if (data == nullptr || outResult == nullptr || count == 0) {
        return false;
    }
    double maxVal = data[0];
    for (std::size_t i = 1; i < count; ++i) {
        if (data[i] > maxVal) {
            maxVal = data[i];
        }
    }
    *outResult = maxVal;
    return true;
}

MATH_DLL_API bool MATH_CALL ArrayAverage(const double* data, std::size_t count, double* outResult) {
    if (data == nullptr || outResult == nullptr || count == 0) {
        return false;
    }
    double sum = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        sum += data[i];
    }
    *outResult = sum / static_cast<double>(count);
    return true;
}

} // extern "C"
