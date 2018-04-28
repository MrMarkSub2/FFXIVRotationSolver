#pragma once

#ifdef ROTATION_SOLVER_EXPORTS
#define ROTATION_SOLVER_API __declspec(dllexport)
#else
#define ROTATION_SOLVER_API __declspec(dllimport)
#endif

#include <memory>
#include "NEATGenome.h"
#include "rdm_moves.h"

class RDMSolver_t {
public:
	ROTATION_SOLVER_API RDMSolver_t();
	ROTATION_SOLVER_API RDMSolver_t(const RDMSolver_t&) = delete;
	void ROTATION_SOLVER_API operator=(const RDMSolver_t&) = delete;
	ROTATION_SOLVER_API ~RDMSolver_t();

	int ROTATION_SOLVER_API getGeneration() const;
	MoveStates_t evaluate(const NEAT::Genome_t& g, const MoveStates_t& input) const;
	void ROTATION_SOLVER_API evaluateGeneration(const MoveStates_t & opener);
	const NEAT::Genome_t ROTATION_SOLVER_API & getBestOfGeneration() const;
	void ROTATION_SOLVER_API nextGeneration();

private:
	struct Impl;
	std::unique_ptr<Impl> pimpl;
};