#include "stdafx.h"
#include "NEATGenome.h"
#include "SparseMatrix.h"
#include "rdm.h"
#include <iostream>

enum RDM_INDEX {
	RDM_INPUT_BEGIN = 0,
	RDM_INPUT_WHITE_MANA = RDM_INPUT_BEGIN, // be careful, this may need to be 101 inputs for each value, a single 0.0 to 1.0 range might not be smart enuf
	RDM_INPUT_BLACK_MANA,

	RDM_INPUT_REMAIN_GCD,
	RDM_INPUT_REMAIN_CASTING,
	RDM_INPUT_REMAIN_ANI_LOCK,
	RDM_INPUT_REMAIN_OUT_OF_RANGE,

	RDM_INPUT_STATUS_DUALCAST,
	RDM_INPUT_STATUS_IMPACT,
	RDM_INPUT_STATUS_VERSTONE,
	RDM_INPUT_STATUS_VERFIRE,
	RDM_INPUT_STATUS_SCATTER,
	RDM_INPUT_STATUS_ACCEL,
	RDM_INPUT_STATUS_EMBOLDEN,
	RDM_INPUT_STATUS_SWIFTCAST,
	RDM_INPUT_STATUS_INFUSION,

	RDM_INPUT_RECAST_CORPS,
	RDM_INPUT_RECAST_DISPLACE,
	RDM_INPUT_RECAST_ACCEL,
	RDM_INPUT_RECAST_MANIFICATION,
	RDM_INPUT_RECAST_EMBOLDEN,
	RDM_INPUT_RECAST_SWIFTCAST,
	RDM_INPUT_RECAST_FLECHE,
	RDM_INPUT_RECAST_CONTRE,
	RDM_INPUT_RECAST_INFUSION,

	RDM_INPUT_MELEE_RIPOSTE,
	RDM_INPUT_MELEE_ZWER,
	RDM_INPUT_MELEE_REDOUBLEMENT,
	RDM_INPUT_MELEE_FINISHER,
	RDM_INPUT_END = RDM_INPUT_MELEE_FINISHER,

	RDM_BIAS,

	RDM_OUTPUT_BEGIN,
	RDM_OUTPUT_JOLT2 = RDM_OUTPUT_BEGIN,
	RDM_OUTPUT_IMPACT,
	RDM_OUTPUT_VERTHUNDER,
	RDM_OUTPUT_VERAERO,
	RDM_OUTPUT_VERFIRE,
	RDM_OUTPUT_VERSTONE,
	RDM_OUTPUT_VERFLARE,
	RDM_OUTPUT_VERHOLY,

	RDM_OUTPUT_CORPS,
	RDM_OUTPUT_DISPLACEMENT,

	RDM_OUTPUT_ACCEL,
	RDM_OUTPUT_MANIFICATION,
	RDM_OUTPUT_EMBOLDEN,
	RDM_OUTPUT_SWIFTCAST,
	RDM_OUTPUT_INFUSION,

	RDM_OUTPUT_FLECHE,
	RDM_OUTPUT_CONTRE,

	RDM_OUTPUT_RIPOSTE,
	RDM_OUTPUT_ENHRIPOSTE,
	RDM_OUTPUT_ZWER,
	RDM_OUTPUT_ENHZWER,
	RDM_OUTPUT_REDOUBLEMENT,
	RDM_OUTPUT_ENHREDOUBLEMENT,
	RDM_OUTPUT_SCATTER,
	RDM_OUTPUT_ENHSCATTER,
	RDM_OUTPUT_MOULINET,
	RDM_OUTPUT_ENHMOULINET,
	RDM_OUTPUT_END = RDM_OUTPUT_ENHMOULINET
};

struct RDMSolver_t::Impl {
	void initialize();
	void performOnePass(std::vector<double>& state, std::vector<double>& state_prev, const SparseMatrix_t<double>& Wxh, const SparseMatrix_t<double>& Whh);
	std::vector<double> getInputArray(const MoveStates_t& movestates);
	int getOutput(const std::vector<double>& state);

	int m_best_of_generation_id;
	NEAT::Population_t m_neat;
	std::map<int, std::shared_ptr<Move_t>> m_moves;
};

void RDMSolver_t::Impl::initialize()
{
	m_best_of_generation_id = -1;

	NEAT::Genome_t no_connect_genome;
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "White Mana");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Black Mana");

	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Remain GCD");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Remain Casting");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Remain Animation Lock");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Remain Out of Range");

	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Dualcast");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Impact");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Verstone");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Verfire");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Scatter");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Acceleration");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Embolden");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Swfitcast");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Status Infusion");

	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Corps-a-corps");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Displacement");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Acceleration");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Manification");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Embolden");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Swiftcast");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Fleche");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Contre Sixte");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Recast Infusion");

	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Melee Combo Riposte");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Melee Combo Zwer");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Melee Combo Redoublement");
	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "Melee Combo Finisher");

	no_connect_genome.addNodeGene(NEAT::INPUT_NODE, "BIAS");

	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Jolt 2");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Impact");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Verthunder");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Veraero");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Verfire");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Verstone");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Verflare");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Verholy");

	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Corps-a-corps");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Displacement");

	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Acceleration");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Manification");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Embolden");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Swiftcast");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Infusion");

	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Fleche");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Contre Sixte");

	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Riposte");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Enh Riposte");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Zwer");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Enh Zwer");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Redoublement");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Enh Redoublement");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Scatter");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Enh Scatter");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Moulinet");
	no_connect_genome.addNodeGene(NEAT::OUTPUT_NODE, "Enh Moulinet");

	std::cout << "Node count: " << no_connect_genome.Whh().rowCount() << std::endl;

	std::map<int, NEAT::Genome_t> representatives;

	for (int i = 0; i < NEAT::Population_t::genome_count; ++i) {
		NEAT::Genome_t mutated_genome(no_connect_genome);

		mutated_genome.addConnectionGene(NEAT::randInt(RDM_INPUT_BEGIN, RDM_INPUT_END), NEAT::randInt(RDM_OUTPUT_BEGIN, RDM_OUTPUT_END), NEAT::getNormalizedRand());

		m_neat.addToCorrectSpecies(mutated_genome, representatives);
	}

	std::cout << "Genome count: " << m_neat.size() << std::endl;
}

std::vector<double> RDMSolver_t::Impl::getInputArray(const MoveStates_t & movestates)
{
	std::vector<double> rval(RDM_INPUT_END + 1, 0.0);
	const State_t& state = movestates.constLastState();




	return rval;
}

RDMSolver_t::RDMSolver_t()
	: pimpl(new Impl())
{
	pimpl->initialize();
}
RDMSolver_t::~RDMSolver_t() = default;

int RDMSolver_t::getGeneration() const
{
	return pimpl->m_neat.getGeneration();
}

MoveStates_t RDMSolver_t::evaluate(const NEAT::Genome_t & g, const MoveStates_t & in_states) const
{
	SparseMatrix_t<double> Wxh = g.Wxh();
	SparseMatrix_t<double> Whh = g.Whh();

	// these are guaranteed to be square matrices
	int matrix_size = (int)Wxh.rowCount();

	//SparseMatrix_t<double> output_w(matrix_size, matrix_size);
	//output_w.addElem(1.0, 3, 3);

	// construct state vector
	std::vector<double> state(matrix_size, 0.0);
	std::vector<double> state_prev(matrix_size, 0.0);

	MoveStates_t movestates = in_states;

	do {
		std::vector<double> inputArray = pimpl->getInputArray(movestates);

		for (int i = 0; i < inputArray.size(); ++i) {
			state[i] += inputArray[i];
		}

		state[RDM_BIAS] = 1; // bias

		pimpl->performOnePass(state, state_prev, Wxh, Whh);

		//std::vector<double> outputArray = pimpl->getOutputArray(state);
		//outputArray = NEAT::softmax(outputArray);
		//int nextMoveId = pimpl->findBestMoveId(outputArray);
		int nextMoveId = pimpl->getOutput(state);
		std::shared_ptr<Move_t> nextMove = pimpl->m_moves[nextMoveId];

		if (movestates.addMove(nextMove) != State_t::UA_NOT_USEABLE) {
			// I guess I don't HAVE to check this?
		}

		movestates.advance(10);
	} while (!movestates.isFinished());

	return movestates;
}

void RDMSolver_t::evaluateGeneration(const MoveStates_t& opener)
{
	for (int g = 0; g < pimpl->m_neat.size(); ++g) {
		const NEAT::Genome_t& genome = pimpl->m_neat.getGenome(g);

		MoveStates_t output = evaluate(genome, opener);
		double fitness = output.constLastState().getDPS().calc();

		//pimpl->m_neat.setFitness(g, fitness);
		pimpl->m_neat.setFitness(g, fitness*fitness);
	}
}

const NEAT::Genome_t& RDMSolver_t::getBestOfGeneration() const
{
	if (pimpl->m_best_of_generation_id == -1)
		pimpl->m_best_of_generation_id = pimpl->m_neat.getFittestGenomeId();

	return pimpl->m_neat.getGenome(pimpl->m_best_of_generation_id);
}

void RDMSolver_t::nextGeneration()
{
	pimpl->m_neat = pimpl->m_neat.createNextGeneration();
	pimpl->m_best_of_generation_id = -1;
}
