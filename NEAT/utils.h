#pragma once

#ifdef NEAT_EXPORTS
#define NEAT_API __declspec(dllexport)
#else
#define NEAT_API __declspec(dllimport)
#endif

namespace NEAT {
	bool NEAT_API IsEssentiallyEqual(double a, double b);
}