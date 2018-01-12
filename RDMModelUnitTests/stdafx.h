// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Headers for CppUnitTest
#include "CppUnitTest.h"

namespace TestCases
{
    std::wstring doublesNotEqualErr(double val, double answer);
    void AssertIsEssentuallyEqual(double val, double answer);
    wchar_t* convert_to_wc(const char* str);
}
