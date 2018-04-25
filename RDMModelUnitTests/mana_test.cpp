#include "stdafx.h"
#include "CppUnitTest.h"
#include "mana.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RDMModelTestCases
{		
	TEST_CLASS(ManaTests)
	{
	public:
		
		TEST_METHOD(Constructor)
		{
            Mana_t m(5, 13);
            Assert::AreEqual(5, m.white());
            Assert::AreEqual(13, m.black());
		}

        TEST_METHOD(BasicAdd)
        {
            Mana_t m(7, 7);
            m.add(6, 12);
            Assert::AreEqual(13, m.white());
            Assert::AreEqual(19, m.black());
        }

        TEST_METHOD(ChainAdd)
        {
            Mana_t m(62, 54);
            m.add(3, 3).add(0, 11);
            Assert::AreEqual(65, m.white());
            Assert::AreEqual(68, m.black());
        }

        TEST_METHOD(Unbalanced)
        {
            Mana_t m(30, 0);
            m.add(3, 2);
            Assert::AreEqual(33, m.white());
            Assert::AreEqual(2, m.black());

            m.add(3, 2);
            Assert::AreEqual(36, m.white());
            Assert::AreEqual(3, m.black());
        }

        TEST_METHOD(Overflow)
        {
            Mana_t m(98, 98);
            m.add(2, 0);
            Assert::AreEqual(100, m.white());
            Assert::AreEqual(98, m.black());

            m.add(2, 0);
            Assert::AreEqual(100, m.white());
            Assert::AreEqual(98, m.black());
        }
	};
}