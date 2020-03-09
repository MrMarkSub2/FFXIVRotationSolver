#include "stdafx.h"
#include "CppUnitTest.h"
#include "../NEAT/utils.h"
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NEATUnitTests
{
	TEST_CLASS(UtilsTests)
	{
	public:

		TEST_METHOD(IsEssentialEqualsTest)
		{
			Assert::IsTrue(NEAT::IsEssentiallyEqual(3.0, 3.0), L"Basic compare");
			Assert::IsTrue(NEAT::IsEssentiallyEqual(1.0, 1.0 / 7 * 7), L"Basic rounding error");
			Assert::IsFalse(NEAT::IsEssentiallyEqual(5.0, 5.1), L"Distinct numbers");
		}

		TEST_METHOD(ReLUTest)
		{
			AssertIsEssentiallyEqual(0.0, NEAT::ReLU(-1), L"Negative");
			AssertIsEssentiallyEqual(0.0, NEAT::ReLU(0), L"Zero");
			AssertIsEssentiallyEqual(5.0, NEAT::ReLU(5), L"Positive");
		}

		TEST_METHOD(softmaxTest)
		{
			std::vector<double> v = { 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0 };
			NEAT::softmax(v);

			// from WolframAlpha
			double denom = 2 + 2 * exp(1) + 2 * pow(exp(1),2) + pow(exp(1),3);

			AssertIsEssentiallyEqual(1 / denom, v[0], L"Mismatch at position 0");
			AssertIsEssentiallyEqual(exp(1) / denom, v[1], L"Mismatch at position 1");
			AssertIsEssentiallyEqual(exp(2) / denom, v[2], L"Mismatch at position 2");
			AssertIsEssentiallyEqual(exp(3) / denom, v[3], L"Mismatch at position 3");
			AssertIsEssentiallyEqual(1 / denom, v[4], L"Mismatch at position 4");
			AssertIsEssentiallyEqual(exp(1) / denom, v[5], L"Mismatch at position 5");
			AssertIsEssentiallyEqual(exp(2) / denom, v[6], L"Mismatch at position 6");
		}
	};
}
