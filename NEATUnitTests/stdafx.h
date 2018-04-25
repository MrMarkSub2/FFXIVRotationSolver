// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Headers for CppUnitTest
#include "CppUnitTest.h"

namespace NEATUnitTests
{
	std::wstring doublesNotEqualErr(double val, double answer, std::wstring err);
	std::wstring doublesEqualErr(double val, double answer, std::wstring err);
	void AssertIsEssentiallyEqual(double answer, double val, std::wstring err);
	void AssertIsEssentiallyEqual(double answer, double val);
	void AssertIsNotEssentiallyEqual(double answer, double val, std::wstring err);
}
