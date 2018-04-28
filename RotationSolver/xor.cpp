#include "stdafx.h"
#include "NEATGenome.h"
#include "SparseMatrix.h"
#include "xor.h"

struct xorSolver_t::Impl {
	void initialize();
	void performOnePass(std::vector<double>& state, std::vector<double>& state_prev, const SparseMatrix_t<double>& Wxh, const SparseMatrix_t<double>& Whh);
	std::vector<double> getOutput(const std::vector<double>& state, const SparseMatrix_t<double>& output_w);

	int m_best_of_generation_id;
	NEAT::Population_t m_neat;
};

void xorSolver_t::Impl::initialize()
{
	m_best_of_generation_id = -1;

	NEAT::Genome_t no_connect_genome;
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Input #1");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Input #2");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "BIAS");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Output");

	std::map<int, NEAT::Genome_t> representatives;

	for (int i = 0; i < NEAT::Population_t::genome_count; ++i) {
		NEAT::Genome_t mutated_genome(no_connect_genome);
		//mutated_genome.addConnectionGene(NEAT::randInt(0, 3), NEAT::randInt(0, 3), NEAT::getNormalizedRand());
		
		//mutated_genome.addConnectionGene(0, 3, NEAT::getNormalizedRand());
		//mutated_genome.addConnectionGene(1, 3, NEAT::getNormalizedRand());
		//mutated_genome.addConnectionGene(2, 3, NEAT::getNormalizedRand());

		mutated_genome.addConnectionGene(NEAT::randInt(0, 2), 3, NEAT::getNormalizedRand());

		m_neat.addToCorrectSpecies(mutated_genome, representatives);
	}
}

void xorSolver_t::Impl::performOnePass(std::vector<double>& state, std::vector<double>& state_prev, const SparseMatrix_t<double>& Wxh, const SparseMatrix_t<double>& Whh)
{
	std::vector<double> curr_w = Wxh.mult(state);
	std::vector<double> prev_w = Whh.mult(state_prev);

	state_prev = state;

	for (int i = 0; i < state.size(); ++i) {
		state[i] = tanh(curr_w[i] + prev_w[i]);
	}
}

std::vector<double> xorSolver_t::Impl::getOutput(const std::vector<double>& state, const SparseMatrix_t<double>& output_w)
{
	std::vector<double> output;
	output.push_back(1 / (1 + exp(-4.9*state[3])));
	//output.push_back(state[3]);
	//return NEAT::softmax(output);
	return output;
}

xorSolver_t::xorSolver_t()
	: pimpl(new Impl())
{
	pimpl->initialize();
}

xorSolver_t::~xorSolver_t() = default;

int xorSolver_t::getGeneration() const
{
	return pimpl->m_neat.getGeneration();
}

std::vector<double> xorSolver_t::evaluate(const NEAT::Genome_t & g, std::vector<double> input) const
{
	SparseMatrix_t<double> Wxh = g.Wxh();
	SparseMatrix_t<double> Whh = g.Whh();

	// these are guaranteed to be square matrices
	int matrix_size = (int)Wxh.rowCount();

	SparseMatrix_t<double> output_w(matrix_size, matrix_size);
	output_w.addElem(1.0, 3, 3);

	// construct state vector
	std::vector<double> state(matrix_size, 0.0);
	std::vector<double> state_prev(matrix_size, 0.0);

	double settled;
	int loopCnt = 0;
	do {
		//TODO
		//if (loopCnt == 0) {
			state[0] += input[0];
			state[1] += input[1];
			state[2] = 1; // bias
		//}

		pimpl->performOnePass(state, state_prev, Wxh, Whh);

		settled = 0.0;
		for (int i = 3; i < state.size(); ++i) {
			settled += abs(state[i] - state_prev[i]);
		}

		/*if (isinf(settled) || settled > 1000000000 || settled < -1000000000)
		int foo;*/

	} while (++loopCnt < 20 && settled > 0.1);

	return pimpl->getOutput(state, output_w);
}

void xorSolver_t::evaluateGeneration()
{
	std::vector<int> test_cases = { 0, 1, 2, 3 };

	for (int g = 0; g < pimpl->m_neat.size(); ++g) {
		std::random_shuffle(test_cases.begin(), test_cases.end());
		
		const NEAT::Genome_t& genome = pimpl->m_neat.getGenome(g);

		double fitness = 4.0;

		for (int t = 0; t < test_cases.size(); ++t) {
			std::vector<double> input = { (double)((test_cases[t] & 0x2) >> 1), (double)(test_cases[t] & 0x1) };
			double answer = ((test_cases[t] & 0x2) >> 1) ^ (test_cases[t] & 0x1);

			std::vector<double> output = evaluate(genome, input);
			//fitness -= abs(answer - output[3]) * abs(answer - output[3]);
			fitness -= abs(answer - output[0]);
		}

		//pimpl->m_neat.setFitness(g, fitness);
		pimpl->m_neat.setFitness(g, fitness*fitness);
	}
}

const NEAT::Genome_t & xorSolver_t::getBestOfGeneration() const
{
	if (pimpl->m_best_of_generation_id == -1)
		pimpl->m_best_of_generation_id = pimpl->m_neat.getFittestGenomeId();

	return pimpl->m_neat.getGenome(pimpl->m_best_of_generation_id);
}

void ROTATION_SOLVER_API xorSolver_t::nextGeneration()
{
	pimpl->m_neat = pimpl->m_neat.createNextGeneration();
	pimpl->m_best_of_generation_id = -1;
}
