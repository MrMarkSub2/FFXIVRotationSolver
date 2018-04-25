#include "stdafx.h"
#include "utils.h"
#include <cmath>
#include <chrono>
#include <cfloat>
#include <random>

bool NEAT::IsEssentiallyEqual(double a, double b) {
	// Calculate the difference.
	double diff = abs(a - b);
	a = abs(a);
	b = abs(b);
	// Find the largest
	double largest = (b > a) ? b : a;

	if (diff <= largest * DBL_EPSILON)
		return true;
	return false;
}

namespace {
	static unsigned seed((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
	static std::default_random_engine generator(seed);
}

double NEAT::getNormalizedRand(double mean, double stdev) {
	std::normal_distribution<double> distribution(mean, stdev);
	return distribution(generator);
}

double NEAT::randDouble(double min, double max) {
	std::uniform_real_distribution<double> distribution(min, max);
	return distribution(generator);
}

int NEAT::randInt(int min, int max) {
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(generator);
}