#pragma once

#ifdef ROTATION_SOLVER_EXPORTS
#define ROTATION_SOLVER_API __declspec(dllexport)
#else
#define ROTATION_SOLVER_API __declspec(dllimport)
#endif

#include <memory>
#include "NEATGenome.h"

class xorSolver_t {
public:
	ROTATION_SOLVER_API xorSolver_t();
	ROTATION_SOLVER_API xorSolver_t(const xorSolver_t&) = delete;
	void ROTATION_SOLVER_API operator=(const xorSolver_t&) = delete;
	ROTATION_SOLVER_API ~xorSolver_t();

	int ROTATION_SOLVER_API getGeneration() const;
	std::vector<double> evaluate(const NEAT::Genome_t& g, std::vector<double> input) const;
	void ROTATION_SOLVER_API evaluateGeneration();
	const NEAT::Genome_t ROTATION_SOLVER_API & getBestOfGeneration() const;
	void ROTATION_SOLVER_API nextGeneration();

private:
	struct Impl;
	std::unique_ptr<Impl> pimpl;
};