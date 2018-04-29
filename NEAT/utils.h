#pragma once

#ifdef NEAT_EXPORTS
#define NEAT_API __declspec(dllexport)
#else
#define NEAT_API __declspec(dllimport)
#endif

#include <vector>

namespace NEAT {
	bool NEAT_API IsEssentiallyEqual(double a, double b);

	// gaussian distribution, mean = 0.0, stdev = 1.0
	double NEAT_API getNormalizedRand(double mean = 0.0, double stdev = 1.0);

	double NEAT_API randDouble(double min = 0.0, double max = 1.0);

	int NEAT_API randInt(int min = 0, int max = 100);

	// https://en.wikipedia.org/wiki/Rectifier_(neural_networks)
	double NEAT_API ReLU(double x);

	// NEAT paper recommended sigmoidal(x) = 1 / (1+e^(?4.9*x)) instead of ReLU, but other sources implied ReLU is really good... maybe experiment

	// consider templatizing
	void NEAT_API softmax(std::vector<double>& v);
}