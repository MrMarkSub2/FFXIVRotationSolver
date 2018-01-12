#include "stdafx.h"
#include "NEST.h"

namespace {
    int g_population_size = 150;
    double g_select_rate = 
}

NEST_t::NEST_t()
    :m_generation(0), m_population(g_population_size)
{ }

void NEST_t::nextGeneration()
{
    NESTPopulation_t ngPopulation = m_population.selectBest();

    m_generation++;
}
