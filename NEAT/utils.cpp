#include "stdafx.h"
#include "utils.h"
#include <algorithm>
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

double NEAT::ReLU(double x)
{
	//return 1.0 / (1 + exp(-4.9*x));
	//return 1.0 / (1 + exp(-x));
	return max(0, x);
	//return tanh(x);
}

void NEAT::softmax(std::vector<double>& v)
{
	// find max element
	double v_max = std::numeric_limits<double>::lowest();
	std::for_each(v.begin(), v.end(), [&](double x) { if (x > v_max) v_max = x; });

	// shift so that biggest value is v_max. Since we take these to e^x, 
	std::transform(v.begin(), v.end(), v.begin(), [=](double x) { return x - v_max; });

	// take everything e^x, and also record the sum
	double v_sum = 0.0;
	std::transform(v.begin(), v.end(), v.begin(), 
		[&](double x) { 
		double new_x = exp(x);
		v_sum += new_x;
		return new_x;
	});

	// finally, divide each value e^x / sum(e^x)
	if (v_sum != 0 && !isnan(v_sum) && !isinf(v_sum))
		std::transform(v.begin(), v.end(), v.begin(), [=](double x) { return x / v_sum; });
}
