#include "stdafx.h"
#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"
#include "NEATGenome.h"
#include "utils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			template<> std::wstring ToString<std::set<int>>(const std::set<int>& t) {
				std::wstring rval = L"{ ";
				for (std::set<int>::const_iterator it = t.begin(); it != t.end(); ++it) {
					if (it != t.begin())
						rval += L", ";
					rval += ToString(*it);
				}
				rval += L" }";
				return rval;
			}
		}
	}
}

namespace NEATUnitTests
{
	std::wstring doublesNotEqualErr(double val, double answer, std::wstring err) {
		std::wstringstream rval;
		rval << "Expected " << val << " but got " << answer;
		if (!err.empty())
			rval << " -- " << err;
		rval << std::endl;
		return rval.str();
	}

	std::wstring doublesEqualErr(double val, double answer, std::wstring err) {
		std::wstringstream rval;
		rval << "Did not expect to match " << val;
		if (!err.empty())
			rval << " -- " << err;
		rval << std::endl;
		return rval.str();
	}

	void AssertIsEssentiallyEqual(double answer, double val, std::wstring err) {
		Assert::IsTrue(NEAT::IsEssentiallyEqual(answer, val), doublesNotEqualErr(answer, val, err).c_str());
	}

	void AssertIsEssentiallyEqual(double answer, double val) {
		Assert::IsTrue(NEAT::IsEssentiallyEqual(answer, val), doublesNotEqualErr(answer, val, L"").c_str());
	}

	void AssertIsNotEssentiallyEqual(double answer, double val, std::wstring err) {
		Assert::IsFalse(NEAT::IsEssentiallyEqual(answer, val), doublesEqualErr(answer, val, err).c_str());
	}

	TEST_CLASS(PopulationTests)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
            Assert::Fail();
		}

	};

	TEST_CLASS(SpeciesTests)
	{
	public:

		TEST_METHOD(TestMethod1)
		{
			Assert::Fail();
		}

	};

	TEST_CLASS(GenomeTests)
	{
	public:

		TEST_METHOD(AddNodesAndConnections)
		{
			NEAT::Genome_t g;
			Assert::AreEqual(0, g.addNodeGene(NEAT::INPUT_NODE, "In #1"), L"In #1");
			Assert::AreEqual(1, g.addNodeGene(NEAT::INPUT_NODE, "In #2"), L"In #2");
			Assert::AreEqual(2, g.addNodeGene(NEAT::INPUT_NODE, "In #3"), L"In #3");
			Assert::AreEqual(3, g.addNodeGene(NEAT::OUTPUT_NODE, "Out #1"), L"Out #1");
			Assert::AreEqual(4, g.addNodeGene(NEAT::HIDDEN_NODE, "Hidden #1"), L"Hidden #1");

			int c1_inno = g.addConnectionGene(0, 3, 1.0);
			Assert::AreEqual(c1_inno + 1, g.addConnectionGene(2, 4, 0.5), L"Connection #2");
			Assert::AreEqual(-1, g.addConnectionGene(2, 4, 10.0), L"Connection #3");
		}

		TEST_METHOD(DisjointExcessTest) {
			NEAT::Genome_t g1;
			Assert::AreEqual(0, g1.addNodeGene(NEAT::INPUT_NODE, "In #1"), L"In #1");
			Assert::AreEqual(1, g1.addNodeGene(NEAT::INPUT_NODE, "In #2"), L"In #2");
			Assert::AreEqual(2, g1.addNodeGene(NEAT::INPUT_NODE, "In #3"), L"In #3");
			Assert::AreEqual(3, g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1"), L"Out #1");
			Assert::AreEqual(4, g1.addNodeGene(NEAT::HIDDEN_NODE, "Hidden #1"), L"Hidden #1");

			std::set<int> mymatch, mydisjoint_lhs, mydisjoint_rhs, myexcess_lhs, myexcess_rhs, mymismatch_lhs, mymismatch_rhs;
			mymatch.insert(g1.addConnectionGene(0, 3, 1.0));
			mymatch.insert(g1.addConnectionGene(1, 3, 1.0));
			mymatch.insert(g1.addConnectionGene(2, 3, 1.0));
			mymatch.insert(g1.addConnectionGene(1, 4, 1.0));
			mymatch.insert(g1.addConnectionGene(4, 3, 1.0));
			
			NEAT::Genome_t g2(g1);
			Assert::AreEqual(5, g2.addNodeGene(NEAT::HIDDEN_NODE, "Hidden #2"), L"Hidden #2");
			mydisjoint_rhs.insert(g2.addConnectionGene(4, 5, 1.0));
			mydisjoint_rhs.insert(g2.addConnectionGene(5, 3, 1.0));
			mydisjoint_lhs.insert(g1.addConnectionGene(0, 4, 1.0));
			myexcess_rhs.insert(g2.addConnectionGene(2, 4, 1.0));
			myexcess_rhs.insert(g2.addConnectionGene(0, 5, 1.0));

			// I can't exactly match disjointed and excess, since these connections might be defined in another testcase
			mymismatch_lhs.insert(mydisjoint_lhs.begin(), mydisjoint_lhs.end());
			mymismatch_lhs.insert(myexcess_lhs.begin(), myexcess_lhs.end());
			mymismatch_rhs.insert(mydisjoint_rhs.begin(), mydisjoint_rhs.end());
			mymismatch_rhs.insert(myexcess_rhs.begin(), myexcess_rhs.end());

			std::set<int> match, disjoint_lhs, disjoint_rhs, excess_lhs, excess_rhs, mismatch_lhs, mismatch_rhs;
			g1.calculateDisjointExcess(g2, match, disjoint_lhs, disjoint_rhs, excess_lhs, excess_rhs);
			// I can't exactly match disjointed and excess, since these connections might be defined in another testcase
			mismatch_lhs.insert(disjoint_lhs.begin(), disjoint_lhs.end());
			mismatch_lhs.insert(excess_lhs.begin(), excess_lhs.end());
			mismatch_rhs.insert(disjoint_rhs.begin(), disjoint_rhs.end());
			mismatch_rhs.insert(excess_rhs.begin(), excess_rhs.end());

			Assert::AreEqual(mymatch, match, L"match mismatch");
			Assert::AreEqual(mymismatch_lhs, mismatch_lhs, L"lhs mismatch mismatch");
			Assert::AreEqual(mymismatch_rhs, mismatch_rhs, L"rhs mismatch mismatch");

			// now try the inverse
			match.clear(); disjoint_lhs.clear(); disjoint_rhs.clear(); excess_lhs.clear(); excess_rhs.clear(); mismatch_lhs.clear(); mismatch_rhs.clear();
			g2.calculateDisjointExcess(g1, match, disjoint_lhs, disjoint_rhs, excess_lhs, excess_rhs);
			// I can't exactly match disjointed and excess, since these connections might be defined in another testcase
			mismatch_lhs.insert(disjoint_lhs.begin(), disjoint_lhs.end());
			mismatch_lhs.insert(excess_lhs.begin(), excess_lhs.end());
			mismatch_rhs.insert(disjoint_rhs.begin(), disjoint_rhs.end());
			mismatch_rhs.insert(excess_rhs.begin(), excess_rhs.end());

			Assert::AreEqual(mymatch, match, L"backwards match mismatch");
			Assert::AreEqual(mymismatch_rhs, mismatch_lhs, L"lhs mismatch mismatch");
			Assert::AreEqual(mymismatch_lhs, mismatch_rhs, L"rhs mismatch mismatch");
		}

		TEST_METHOD(OffspringTest) {
			NEAT::Genome_t g1;
			Assert::AreEqual(0, g1.addNodeGene(NEAT::INPUT_NODE, "In #1"), L"In #1");
			Assert::AreEqual(1, g1.addNodeGene(NEAT::INPUT_NODE, "In #2"), L"In #2");
			Assert::AreEqual(2, g1.addNodeGene(NEAT::INPUT_NODE, "In #3"), L"In #3");
			Assert::AreEqual(3, g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1"), L"Out #1");
			Assert::AreEqual(4, g1.addNodeGene(NEAT::HIDDEN_NODE, "Hidden #1"), L"Hidden #1");

			g1.addConnectionGene(0, 3, 0.7);
			g1.addConnectionGene(1, 3, 0.5);
			g1.addConnectionGene(2, 3, 0.5);
			g1.addConnectionGene(1, 4, 0.2);
			g1.addConnectionGene(4, 3, 0.4);

			NEAT::Genome_t g2(g1);
			Assert::AreEqual(5, g2.addNodeGene(NEAT::HIDDEN_NODE, "Hidden #2"), L"Hidden #2");
			Assert::IsTrue(g2.updateConnectionGene(0, 3, 0.8));
			Assert::IsTrue(g2.updateConnectionGene(1, 3, 0.3));
			Assert::IsTrue(g2.updateConnectionGene(2, 3, 0.8));
			Assert::IsTrue(g2.updateConnectionGene(1, 4, 0.6));
			Assert::IsTrue(g2.updateConnectionGene(4, 3, 0.9));

			g2.addConnectionGene(4, 5, 0.1);
			g2.addConnectionGene(5, 3, 0.2);
			g1.addConnectionGene(0, 4, 0.3);
			g2.addConnectionGene(2, 4, 0.4);
			g2.addConnectionGene(0, 5, 0.5);

			AssertIsEssentiallyEqual(304.0 / 450.0, g1.delta(g2), L"g1.delta(g2)");
			AssertIsEssentiallyEqual(304.0 / 450.0, g2.delta(g1), L"g2.delta(g1)");

			NEAT::Genome_t g3(g1.makeOffspring(g2));
			AssertIsNotEssentiallyEqual(304.0 / 450.0, g3.delta(g1), L"g3.delta(g1)");
			AssertIsNotEssentiallyEqual(304.0 / 450.0, g3.delta(g2), L"g3.delta(g2)");
		}

		TEST_METHOD(DeltaTest) {
			NEAT::Genome_t g1;
			Assert::AreEqual(0, g1.addNodeGene(NEAT::INPUT_NODE, "In #1"), L"In #1");
			Assert::AreEqual(1, g1.addNodeGene(NEAT::INPUT_NODE, "In #2"), L"In #2");
			Assert::AreEqual(2, g1.addNodeGene(NEAT::INPUT_NODE, "In #3"), L"In #3");
			Assert::AreEqual(3, g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1"), L"Out #1");
			Assert::AreEqual(4, g1.addNodeGene(NEAT::HIDDEN_NODE, "Hidden #1"), L"Hidden #1");

			g1.addConnectionGene(0, 3, 0.7);
			g1.addConnectionGene(1, 3, 0.5);
			g1.addConnectionGene(2, 3, 0.5);
			g1.addConnectionGene(1, 4, 0.2);
			g1.addConnectionGene(4, 3, 0.4);

			NEAT::Genome_t g2(g1);
			Assert::AreEqual(5, g2.addNodeGene(NEAT::HIDDEN_NODE, "Hidden #2"), L"Hidden #2");
			Assert::IsTrue(g2.updateConnectionGene(0, 3, 0.8));
			Assert::IsTrue(g2.updateConnectionGene(1, 3, 0.3));
			Assert::IsTrue(g2.updateConnectionGene(2, 3, 0.8));
			Assert::IsTrue(g2.updateConnectionGene(1, 4, 0.6));
			Assert::IsTrue(g2.updateConnectionGene(4, 3, 0.9));

			g2.addConnectionGene(4, 5, 0.1);
			g2.addConnectionGene(5, 3, 0.2);
			g1.addConnectionGene(0, 4, 0.3);
			g2.addConnectionGene(2, 4, 0.4);
			g2.addConnectionGene(0, 5, 0.5);

			AssertIsEssentiallyEqual(304.0 / 450.0, g1.delta(g2), L"g1.delta(g2)");
			AssertIsEssentiallyEqual(304.0 / 450.0, g2.delta(g1), L"g2.delta(g1)");
		}


		TEST_METHOD(WxhTest) {
			NEAT::Genome_t g;
			Assert::AreEqual(0, g.addNodeGene(NEAT::INPUT_NODE, "In #1"), L"In #1");
			Assert::AreEqual(1, g.addNodeGene(NEAT::INPUT_NODE, "In #2"), L"In #2");
			Assert::AreEqual(2, g.addNodeGene(NEAT::INPUT_NODE, "In #3"), L"In #3");
			Assert::AreEqual(3, g.addNodeGene(NEAT::OUTPUT_NODE, "Out #1"), L"Out #1");
			Assert::AreEqual(4, g.addNodeGene(NEAT::HIDDEN_NODE, "Hidden #1"), L"Hidden #1");

			g.addConnectionGene(0, 3, 0.7);
			g.addConnectionGene(2, 3, 0.5);
			g.addConnectionGene(1, 4, 0.2);
			g.addConnectionGene(4, 3, 0.4);
			g.addConnectionGene(0, 4, 0.6);
			g.addConnectionGene(3, 4, 0.6);

			std::vector<double> s0;
			try {
				s0 = g.Wxh().mult({ 1, 1, 1, 0, 0 });
			}
			catch (std::runtime_error&) {
				Assert::Fail(L"s0 multiplication failed!");
			}

			AssertIsEssentiallyEqual(0.0, s0[0]);
			AssertIsEssentiallyEqual(0.0, s0[1]);
			AssertIsEssentiallyEqual(0.0, s0[2]);
			AssertIsEssentiallyEqual(1.52, s0[3]);
			AssertIsEssentiallyEqual(0.8, s0[4]);

			std::vector<double> s1;
			try {
				s1 = addVectors(g.Wxh().mult({ 1,1,1,0,0 }), g.Whh().mult(s0));
			}
			catch (std::runtime_error&) {
				Assert::Fail(L"s1 multiplication failed!");
			}

			AssertIsEssentiallyEqual(0.0, s1[0]);
			AssertIsEssentiallyEqual(0.0, s1[1]);
			AssertIsEssentiallyEqual(0.0, s1[2]);
			AssertIsEssentiallyEqual(1.52, s1[3]);
			AssertIsEssentiallyEqual(1.712, s1[4]);
		}
	};

	TEST_CLASS(NodeGeneTests)
	{
	public:

		TEST_METHOD(NodeGeneTestIGuess)
		{
			NEAT::NodeGene_t ng1(5, NEAT::INPUT_NODE, "Test gene");
			Assert::AreEqual(std::string("Test gene"), ng1.getLabel());
		}

	};

	TEST_CLASS(ConnectionNodeTests)
	{
	public:

		TEST_METHOD(Incremental)
		{
			NEAT::ConnectionGene_t c1(1, 2, 5.0);
			int c1_inno = c1.getInnovationNumber();

			NEAT::ConnectionGene_t c2(3, 4, 17.0);
			Assert::AreEqual(c1_inno + 1, c2.getInnovationNumber());

			NEAT::ConnectionGene_t c3(c2);
			Assert::AreEqual(c2.getInnovationNumber(), c3.getInnovationNumber());
		}

		TEST_METHOD(EquivalentConnectionCheck) {
			NEAT::ConnectionGene_t c1(1, 2, 5.0);
			NEAT::ConnectionGene_t c2(3, 4, 17.0);
			NEAT::ConnectionGene_t c3(3, 4, 92.0);

			Assert::IsFalse(c1.isEquivalentConnection(c2));
			Assert::IsTrue(c2.isEquivalentConnection(c3));
			Assert::IsFalse(c3.isEquivalentConnection(c1));
		}

	};

}
