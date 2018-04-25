#pragma once

#ifdef NEAT_EXPORTS
#define NEAT_API __declspec(dllexport)
#else
#define NEAT_API __declspec(dllimport)
#endif

namespace NEAT {
	bool NEAT_API IsEssentiallyEqual(double a, double b);

	// gaussian distribution, mean = 0.0, stdev = 1.0
	double NEAT_API getNormalizedRand(double mean = 0.0, double stdev = 1.0);

	double NEAT_API randDouble(double min = 0.0, double max = 1.0);

	int NEAT_API randInt(int min = 0, int max = 100);
}