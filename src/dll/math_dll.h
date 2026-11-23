#pragma once

#include <cstddef>

#ifdef KHLUS_MATH_DLL_EXPORTS
    #define MATH_DLL_API __declspec(dllexport)
#else
    #define MATH_DLL_API __declspec(dllimport)
#endif

#define MATH_CALL __cdecl

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Computes the sum of array elements.
 * @param data      Pointer to the input array of doubles.
 * @param count     Number of elements in the array (must be > 0).
 * @param outResult Pointer to receive the computed sum.
 * @return true on success, false if arguments are invalid.
 */
MATH_DLL_API bool MATH_CALL ArraySum(const double* data, std::size_t count, double* outResult);

/**
 * Finds the minimum element in the array.
 * @param data      Pointer to the input array of doubles.
 * @param count     Number of elements in the array (must be > 0).
 * @param outResult Pointer to receive the minimum value.
 * @return true on success, false if arguments are invalid.
 */
MATH_DLL_API bool MATH_CALL ArrayMin(const double* data, std::size_t count, double* outResult);

/**
 * Finds the maximum element in the array.
 * @param data      Pointer to the input array of doubles.
 * @param count     Number of elements in the array (must be > 0).
 * @param outResult Pointer to receive the maximum value.
 * @return true on success, false if arguments are invalid.
 */
MATH_DLL_API bool MATH_CALL ArrayMax(const double* data, std::size_t count, double* outResult);

/**
 * Computes the arithmetic mean (average) of array elements.
 * @param data      Pointer to the input array of doubles.
 * @param count     Number of elements in the array (must be > 0).
 * @param outResult Pointer to receive the computed average.
 * @return true on success, false if arguments are invalid.
 */
MATH_DLL_API bool MATH_CALL ArrayAverage(const double* data, std::size_t count, double* outResult);

#ifdef __cplusplus
}
#endif
