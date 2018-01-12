#include "stdafx.h"
#include "CppUnitTest.h"
#include "NEATGenome.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NEATUnitTests
{		
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

		TEST_METHOD(TestMethod1)
		{
			Assert::Fail();
		}

	};

	TEST_CLASS(NodeGeneTests)
	{
	public:

		TEST_METHOD(NodeGeneTestIGuess)
		{
			NEAT::NodeGene_t ng1(5, NEAT::INPUT_NODE, "Test gene");
			ng1.setValue(4.2);
			Assert::AreEqual(4.2, ng1.getValue());
			Assert::AreEqual(std::string("Test gene"), ng1.getLabel());
		}

	};

	TEST_CLASS(ConnectionNodeTests)
	{
	public:

		TEST_METHOD(Incremental)
		{
			NEAT::ConnectionGene_t c1(1, 2, 5.0);
			Assert::AreEqual(0, c1.getInnovationNumber());

			NEAT::ConnectionGene_t c2(3, 4, 17.0);
			Assert::AreEqual(1, c2.getInnovationNumber());

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
