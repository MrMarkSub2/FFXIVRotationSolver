#include "stdafx.h"
#include "utils.h"
#include <cmath>
#include <cfloat>

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
