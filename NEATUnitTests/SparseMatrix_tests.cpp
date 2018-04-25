#include "stdafx.h"
#include "CppUnitTest.h"
#include "SparseMatrix.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NEATUnitTests
{

	TEST_CLASS(SparseMatrixTests)
	{
	public:

		TEST_METHOD(BasicSparseMatrix)
		{
			SparseMatrix_t<int> matrix(4, 3);
			Assert::AreEqual((size_t)0, matrix.size());
			matrix.addElem(4, 1, 2);
			Assert::AreEqual((size_t)1, matrix.size());
			matrix.addElem(7, 0, 0);
			Assert::AreEqual((size_t)2, matrix.size());
		}

		TEST_METHOD(OOB)
		{
			SparseMatrix_t<int> matrix(4, 3);
			try {
				matrix.addElem(2, 4, 0);
				Assert::Fail(L"matrix insert succeeded despite OOB");
			}
			catch (std::runtime_error&) {
				// good!
			}

			std::vector<int> v1 = { 1, 2 };
			try {
				matrix.mult(v1);
				Assert::Fail(L"matrix multiply succeeded despite size mismatch");
			}
			catch (std::runtime_error&) {
				// good!
			}

			std::vector<int> v2 = { 3, 4, 5 };
			try {
				addVectors(v1, v2);
				Assert::Fail(L"matrix multiply succeeded despite size mismatch");
			}
			catch (std::runtime_error&) {
				// good!
			}
		}

		TEST_METHOD(IntVectorMultiplication) {
			SparseMatrix_t<int> matrix(2, 3);
			matrix.addElem(1, 0, 0);
			matrix.addElem(-1, 0, 1);
			matrix.addElem(-3, 1, 1);
			matrix.addElem(1, 1, 2);

			std::vector<int> v = { 2, 1, 0 };

			std::vector<int> rval = matrix.mult(v);

			Assert::AreEqual(1, rval[0]);
			Assert::AreEqual(-3, rval[1]);
		}

		TEST_METHOD(VectorAddition) {
			std::vector<int> v1 = { 1, 2, 3 };
			std::vector<int> v2 = { 7, 8, 9 };

			std::vector<int> rval = addVectors(v1, v2);

			Assert::AreEqual(8, rval[0]);
			Assert::AreEqual(10, rval[1]);
			Assert::AreEqual(12, rval[2]);
		}

		TEST_METHOD(MatrixAddition) {
			SparseMatrix_t<int> m1(3, 4);
			m1.addElem(5, 0, 2);
			m1.addElem(5, 1, 0);
			m1.addElem(3, 1, 2);
			m1.addElem(1, 2, 1);
			m1.addElem(2, 2, 3);

			SparseMatrix_t<int> m2(3, 4);
			m2.addElem(6, 1, 1);
			m2.addElem(4, 0, 1);
			m2.addElem(3, 2, 1);
			m2.addElem(9, 0, 2);
			m2.addElem(1, 2, 2);

			SparseMatrix_t<int> sum = m1.add(m2);

			SparseMatrix_t<int> answer(3, 4);
			answer.addElem(5, 1, 0);
			answer.addElem(4, 0, 1);
			answer.addElem(6, 1, 1);
			answer.addElem(4, 2, 1);
			answer.addElem(14, 0, 2);
			answer.addElem(3, 1, 2);
			answer.addElem(1, 2, 2);
			answer.addElem(2, 2, 3);

			Assert::IsTrue(sum.isEqual(answer));
		}

		TEST_METHOD(MatrixMultiplication) {
			SparseMatrix_t<int> m1(3, 4);
			m1.addElem(5, 0, 2);
			m1.addElem(5, 1, 0);
			m1.addElem(3, 1, 2);
			m1.addElem(1, 2, 1);
			m1.addElem(2, 2, 3);

			SparseMatrix_t<int> m2(4, 3);
			m2.addElem(6, 1, 1);
			m2.addElem(4, 1, 0);
			m2.addElem(3, 1, 2);
			m2.addElem(9, 2, 0);
			m2.addElem(1, 2, 2);

			SparseMatrix_t<int> product = m1.mult(m2);

			SparseMatrix_t<int> answer(3, 3);
			answer.addElem(45, 0, 0);
			answer.addElem(5, 0, 2);
			answer.addElem(27, 1, 0);
			answer.addElem(3, 1, 2);
			answer.addElem(4, 2, 0);
			answer.addElem(6, 2, 1);
			answer.addElem(3, 2, 2);

			Assert::IsTrue(product.isEqual(answer));
		}

		TEST_METHOD(FullyExpandedMatrix) {
			SparseMatrix_t<double> m1(5, 5);
			m1.addElem(0.7, 3, 0);
			m1.addElem(0.5, 3, 2);
			m1.addElem(0.2, 4, 1);
			m1.addElem(0.4, 3, 4);
			m1.addElem(0.6, 4, 0);
			//m1.addElem(0.6, 4, 3); no recurrent connections!

			SparseMatrix_t<double> m1_expanded = m1.fullyExpand();

			SparseMatrix_t<double> m1_answer(5, 5);
			m1_answer.addElem(0.94, 3, 0);
			m1_answer.addElem(0.08, 3, 1);
			m1_answer.addElem(0.5, 3, 2);
			m1_answer.addElem(0.4, 3, 4);
			m1_answer.addElem(0.6, 4, 0);
			m1_answer.addElem(0.2, 4, 1);

			Assert::IsTrue(m1_expanded.isEqual(m1_answer), L"m1 did not expand properly!");
		}

		TEST_METHOD(MatrixEquality) {
			SparseMatrix_t<int> m1(3, 4);
			m1.addElem(5, 0, 2);
			m1.addElem(5, 1, 0);
			m1.addElem(3, 1, 2);
			m1.addElem(1, 2, 1);
			m1.addElem(2, 2, 3);

			// same matrix, just shuffled entries
			SparseMatrix_t<int> m2(3, 4);
			m2.addElem(3, 1, 2);
			m2.addElem(5, 1, 0);
			m2.addElem(2, 2, 3);
			m2.addElem(5, 0, 2);
			m2.addElem(1, 2, 1);
			
			Assert::IsTrue(m1.isEqual(m2), L"m1 not equal m2 but should be!");
			Assert::IsTrue(m2.isEqual(m1), L"m2 not equal m1 but should be!");

			m2.addElem(7, 0, 0);
			Assert::IsFalse(m1.isEqual(m2), L"m1 equal m2 but shouldn't be!");
			Assert::IsFalse(m2.isEqual(m1), L"m2 equal m1 but shouldn't be!");
		}

	};

}