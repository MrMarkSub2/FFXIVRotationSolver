#pragma once

#include <string>

#ifdef RDM_MODEL_EXPORTS
#define RDM_MODEL_API __declspec(dllexport)
#else
#define RDM_MODEL_API __declspec(dllimport)
#endif

// in milliseconds e.g. 2420
RDM_MODEL_API int calculateGCD(int ss, double huton, double haste, double arrow, double feywind, double UI3AF3);
RDM_MODEL_API bool IsEssentiallyEqual(double a, double b);
RDM_MODEL_API void outputWordWrap(std::string s);