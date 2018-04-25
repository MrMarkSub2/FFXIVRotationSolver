#include "stdafx.h"
#include "CppUnitTest.h"
#include "utils.h"
#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RDMModelTestCases
{
    std::wstring doublesNotEqualErr(double val, double answer) {
        std::wstringstream rval;
        rval << "Expected " << val << " but got " << answer << std::endl;
        return rval.str();
    }

    void AssertIsEssentuallyEqual(double val, double answer) {
        Assert::IsTrue(IsEssentiallyEqual(val, answer), doublesNotEqualErr(val, answer).c_str());
    }

    wchar_t* convert_to_wc(const char* str) {
        size_t newsize = strlen(str) + 1;
        wchar_t * wcstring = new wchar_t[newsize];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wcstring, newsize, str, _TRUNCATE);
        return wcstring;
    }

    TEST_CLASS(UtilsTests)
    {
    public:

        TEST_METHOD(BasicGCD)
        {
            int gcd = calculateGCD(364, 0.0, 0.0, 0.0, 0.0, 1.0);
            Assert::AreEqual(2500, gcd);
        }

        TEST_METHOD(RDM_GCD)
        {
            int gcd = calculateGCD(849, 0.0, 0.0, 0.0, 0.0, 1.0);
            Assert::AreEqual(2420, gcd);
        }

        TEST_METHOD(GottaGoFast)
        {
            int gcd_nin = calculateGCD(364, 0.15, 0.0, 0.0, 0.0, 1.0);
            Assert::AreEqual(2120, gcd_nin);
            int gcd_hasted_nin = calculateGCD(364, 0.15, 0.15, 0.0, 0.0, 1.0);
            Assert::AreEqual(1800, gcd_hasted_nin);
            int gcd_hasted_nin_arrow = calculateGCD(364, 0.15, 0.15, 0.15, 0.0, 1.0);
            Assert::AreEqual(1520, gcd_hasted_nin_arrow);
            int gcd_gdi_scholar = calculateGCD(364, 0.15, 0.15, 0.15, 0.03, 1.0);
            Assert::AreEqual(1450, gcd_gdi_scholar);
            int gcd_fuck_you_im_BLM_now = calculateGCD(364, 0.15, 0.15, 0.15, 0.03, 0.5);
            Assert::AreEqual(720, gcd_fuck_you_im_BLM_now);
        }

        TEST_METHOD(RandomGCD)
        {
            int random_gcd = calculateGCD(1985, 0.05, 0.3, 0.05, 0.0, 1.0);
            Assert::AreEqual(1420, random_gcd);
        }
    };
}