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
		
		TEST_METHOD(SizeTest)
		{
			NEAT::Population_t pop;

			Assert::AreEqual(0, pop.size(), L"Empty population not empty");

			NEAT::Genome_t g1;
			int in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #1");
			int out_num = g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1");
			g1.addConnectionGene(in_num, out_num, 1.0);

			for (int i = 0; i < 20; ++i) {
				pop.addToSpecificSpecies(g1, 0);
				Assert::AreEqual(i + 1, pop.size(), L"Size ain't adding up");
			}
		}

		TEST_METHOD(GenNumTest)
		{
			NEAT::Population_t pop;

			Assert::AreEqual(0, pop.getGeneration(), L"Initial population not Gen #0");

			NEAT::Genome_t g1;
			int in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #1");
			int out_num = g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1");
			g1.addConnectionGene(in_num, out_num, 1.0);

			for (int i = 0; i < 20; ++i) {
				int id = pop.addToSpecificSpecies(g1, 0);
				pop.setFitness(id, id);
			}

			for (int i = 0; i < 5; ++i) {
				pop = pop.createNextGeneration();
				Assert::AreEqual(i+1, pop.getGeneration(), L"Generation number not incrementing");
			}
		}

		TEST_METHOD(GetGenomeBackTest)
		{
			NEAT::Population_t pop;

			NEAT::Genome_t g1;
			int in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #1");
			int out_num = g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1");
			g1.addConnectionGene(in_num, out_num, 1.0);

			pop.addToSpecificSpecies(g1, 0);

			NEAT::Genome_t g2 = pop.getGenome(0);

			AssertIsEssentiallyEqual(0.0, g1.delta(g2), L"delta(a,b) doesn't imply a match");
			Assert::IsTrue(g1.Wxh().isEqual(g2.Wxh()), L"Wxh() doesn't imply a match");
			Assert::IsTrue(g1.Whh().isEqual(g2.Whh()), L"Whh() doesn't imply a match");

			// this should not compile when uncommented. This is supposed to be protected by const!
			// pop.getGenome(0).updateConnectionGene(in_num, out_num, 1.7);

			try {
				pop.getGenome(666);
				Assert::Fail(L"What is this nonexistant genome?!");
			}
			catch (std::runtime_error&) {
				// success
			}
		}

		TEST_METHOD(FitnessTest)
		{
			// Not as bad as it sounds! No water bottle and gym shorts required
			NEAT::Population_t pop;

			NEAT::Genome_t g1;
			int in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #1");
			int out_num = g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1");
			g1.addConnectionGene(in_num, out_num, 1.0);

			std::map<int, NEAT::Genome_t> representatives;
			representatives[0] = g1;

			for (int i = 0; i < 5; ++i)
				pop.addToCorrectSpecies(g1, representatives);

			Assert::AreEqual(1, (int)pop.getSpeciesIds().size(), L"Identical things going into different species???");
			Assert::AreEqual(0, *pop.getSpeciesIds().begin(), L"Representative should have placed us in species #0");

			pop.setFitness(0, 1.0);
			pop.setFitness(1, 5.0);
			pop.setFitness(2, 2.0);
			pop.setFitness(3, 176.0);
			pop.setFitness(4, 14.0);

			pop.addToSpecificSpecies(g1, 3);
			pop.setFitness(5, 500.0);


			Assert::AreEqual(3, pop.getFittestGenomeIdofSpecies(0), L"Species #0 failed to find the fittest");
			Assert::AreEqual(-1, pop.getFittestGenomeIdofSpecies(999), L"Species #999 isn't supposed to exist");
			Assert::AreEqual(5, pop.getFittestGenomeIdofSpecies(3), L"Species #3 failed to find the fittest");

			Assert::AreEqual(5, pop.getFittestGenomeId(), L"Couldn't find fittest across all species");

			std::vector<int> top_half = pop.getBestGenomeIdsOfSpecies(0, 0.4);
			Assert::AreEqual(2, (int)top_half.size(), L"Couldn't find the top 40%");
			Assert::AreEqual(3, top_half[0], L"Couldn't find the highest");
			Assert::AreEqual(4, top_half[1], L"Couldn't find the second highest");
		}

		TEST_METHOD(SpecificSpeciesTest)
		{
			NEAT::Population_t pop;

			NEAT::Genome_t g1;
			int in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #1");
			int out_num = g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1");
			g1.addConnectionGene(in_num, out_num, 1.0);

			pop.addToSpecificSpecies(g1, 5);
			pop.addToSpecificSpecies(g1, 9);
			pop.addToSpecificSpecies(g1, 4);
			pop.addToSpecificSpecies(g1, 9);
			
			std::vector<int> species_ids = pop.getSpeciesIds();
			Assert::AreEqual(3, (int)species_ids.size(), L"Wrong # of species");
			Assert::AreEqual(4, species_ids[0], L"Species ids are incorrect");
			Assert::AreEqual(5, species_ids[1], L"Species ids are incorrect");
			Assert::AreEqual(9, species_ids[2], L"Species ids are incorrect");

			std::vector<int> genome_ids = pop.getGenomeIdsOfSpecies(4);
			Assert::AreEqual(1, (int)genome_ids.size(), L"Wrong # of genomes for species #4");
			Assert::AreEqual(2, genome_ids[0], L"Genome ids are incorrect for species #4");

			genome_ids = pop.getGenomeIdsOfSpecies(5);
			Assert::AreEqual(1, (int)genome_ids.size(), L"Wrong # of genomes for species #5");
			Assert::AreEqual(0, genome_ids[0], L"Genome ids are incorrect for species #5");

			genome_ids = pop.getGenomeIdsOfSpecies(9);
			Assert::AreEqual(2, (int)genome_ids.size(), L"Wrong # of genomes for species #9");
			Assert::AreEqual(1, genome_ids[0], L"Genome ids are incorrect for species #9");
			Assert::AreEqual(3, genome_ids[1], L"Genome ids are incorrect for species #9");
		}

		TEST_METHOD(CorrectSpeciesTest)
		{
			NEAT::Population_t pop, last_pop;

			NEAT::Genome_t g1;
			int in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #1");
			int out_num = g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1");
			g1.addConnectionGene(in_num, out_num, 1.0);

			std::map<int, NEAT::Genome_t> representatives;
			representatives[0] = g1;

			pop.addToCorrectSpecies(g1, representatives);
			Assert::AreEqual(1, (int)pop.getGenomeIdsOfSpecies(0).size(), L"Where did g1 go?");

			// confirm that duplicate genome lands in same species
			pop.addToCorrectSpecies(g1, representatives);
			Assert::AreEqual(2, (int)pop.getGenomeIdsOfSpecies(0).size(), L"g1 and g2 aren't equal?");

			// create a genome that should land in the same species, despite modifications
			NEAT::Genome_t g2(g1);
			g2.addConnectionGene(out_num, out_num, 20.0);
			g2.updateConnectionGene(in_num, out_num, 5.5);
			AssertIsEssentiallyEqual(5.5, g1.delta(g2), L"Okay, now my delta math is way off for g2...");
			// unless I muck around with the globals, a delta of 5.5 should still fall within the same species
			pop.addToCorrectSpecies(g2, representatives);
			Assert::AreEqual(1, (int)pop.getSpeciesIds().size(), L"g2 didn't land in the same species... is NEAT::Population_t::starting_delta_t == 6?");
			Assert::AreEqual(3, (int)pop.getGenomeIdsOfSpecies(0).size(), L"Now where did g2 go?!");

			// create a genome that is divergent due to connection count
			NEAT::Genome_t g3(g2);
			for (int i = 0; i < 10; ++i) {
				int new_num = g3.addNodeGene(NEAT::HIDDEN_NODE, "BLOAT BLOAT BLOAT");
				g3.addConnectionGene(in_num, new_num, 5.0);
			}
			AssertIsEssentiallyEqual(19.0 / 3.0, g1.delta(g3), L"Okay, now my delta math is way off for g3...");
			pop.addToCorrectSpecies(g3, representatives);
			Assert::AreEqual(2, (int)pop.getSpeciesIds().size(), L"g3 should have diverged");
			Assert::AreEqual(2, (int)representatives.size(), L"g3 forgot to update representatives");
			Assert::AreEqual(1, (int)pop.getGenomeIdsOfSpecies(pop.getSpeciesIds()[1]).size(), L"Where is g3?");

			// create a genome that is divergent due to weights
			NEAT::Genome_t g4(g1);
			g4.updateConnectionGene(in_num, out_num, 999.0);
			AssertIsEssentiallyEqual(998.0, g1.delta(g4), L"Okay, now my delta math is way off for g4...");
			AssertIsEssentiallyEqual(993.5 + 11.0 / 6.0, g3.delta(g4), L"Okay, now my delta math is way off for g4 part 2...");
			pop.addToCorrectSpecies(g4, representatives);
			Assert::AreEqual(3, (int)pop.getSpeciesIds().size(), L"g4 should have diverged");
			Assert::AreEqual(3, (int)representatives.size(), L"g4 forgot to update representatives");
			Assert::AreEqual(1, (int)pop.getGenomeIdsOfSpecies(pop.getSpeciesIds()[2]).size(), L"Where is g4?");
		}

		TEST_METHOD(NextGenerationTest)
		{
			// due to rand() calls, I'm limited to what I can accurately test
			NEAT::Population_t pop1;

			NEAT::Genome_t g1;
			int in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #1");
			int out_num = g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1");
			g1.addConnectionGene(in_num, out_num, 1.0);

			for (int i = 0; i < 5; ++i) {
				for (int j = 0; j < NEAT::Population_t::genome_count / 5; ++j) {
					int id = pop1.addToSpecificSpecies(g1, i);
					pop1.setFitness(id, NEAT::Population_t::genome_count / 5 * i + j);
				}
			}
			Assert::AreEqual(NEAT::Population_t::genome_count, pop1.size(), L"Didn't start at correct max population size");

			NEAT::Population_t pop2 = pop1.createNextGeneration();
			Assert::AreEqual(NEAT::Population_t::genome_count, pop2.size(), L"Gained/lost max population size in gen #1");
			Assert::IsTrue(pop2.getSpeciesIds().size() >= pop1.getSpeciesIds().size(), L"Gen #1 somehow lost species... it shouldn't be possible this soon");

			NEAT::Population_t pop3 = pop2.createNextGeneration();
			Assert::AreEqual(NEAT::Population_t::genome_count, pop3.size(), L"Gained/lost max population size in gen #2");

			// This feels sparse but I'm not sure what else to realistically test here
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
			Assert::AreNotEqual(-1, g.addConnectionGene(2, 4, 0.5), L"Connection #2");
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
			g2.addConnectionGene(0, 4, 0.3);
			g2.addConnectionGene(2, 4, 0.4);
			g2.addConnectionGene(0, 5, 0.5);

			AssertIsEssentiallyEqual(1.3, g1.delta(g2), L"g1.delta(g2)");
			AssertIsEssentiallyEqual(1.3, g2.delta(g1), L"g2.delta(g1)");

			NEAT::Genome_t g3(g1.makeOffspring(g2));
			AssertIsNotEssentiallyEqual(1.3, g3.delta(g1), L"g3.delta(g1)");
			AssertIsNotEssentiallyEqual(1.3, g3.delta(g2), L"g3.delta(g2)");
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
			g2.addConnectionGene(0, 4, 0.3);
			g2.addConnectionGene(2, 4, 0.4);
			g2.addConnectionGene(0, 5, 0.5);

			AssertIsEssentiallyEqual(1.3, g1.delta(g2), L"g1.delta(g2)");
			AssertIsEssentiallyEqual(1.3, g2.delta(g1), L"g2.delta(g1)");
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
