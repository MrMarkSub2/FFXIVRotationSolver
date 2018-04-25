#include "stdafx.h"
#include "CppUnitTest.h"
#include "dps.h"
//#include "mana.h"
//#include <cfloat>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RDMModelTestCases
{
    TEST_CLASS(DPSTests)
    {
    public:
        TEST_METHOD(BasicAdd)
        {
            //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
            DPS_t d;
            d.add(5, 5000);
            AssertIsEssentuallyEqual(1.0, d.calc());
        }

        TEST_METHOD(CompoundAdd) 
        {
            DPS_t d;
            d.add(12, 2500).add(13, 7500);
            AssertIsEssentuallyEqual(2.5, d.calc());
        }

        TEST_METHOD(AddAnotherDPS)
        {
            DPS_t d1, d2;
            d1.add(15, 4600).add(26, 7200);
            d2.add(7, 7200).add(167, 1000);
            AssertIsEssentuallyEqual(3.4745762711864406779661016949153, d1.calc());
            AssertIsEssentuallyEqual(21.219512195121951219512195121951, d2.calc());

            d1.add(d2);
            AssertIsEssentuallyEqual(10.75, d1.calc());
        }

        TEST_METHOD(DivideByZero) 
        {
            DPS_t d;
            AssertIsEssentuallyEqual(0.0, d.calc());
        }

        TEST_METHOD(AddFunc)
        {
            DPS_t memberFunc;
            memberFunc.add(16, 1230).add(63, 4620);

            DPS_t helperFunc;
            addDPS(helperFunc, 16, 1230);
            addDPS(helperFunc, 63, 4620);

            AssertIsEssentuallyEqual(memberFunc.calc(), helperFunc.calc());
        }
    };
}