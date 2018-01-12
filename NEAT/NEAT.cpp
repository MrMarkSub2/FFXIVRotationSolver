#include "stdafx.h"
#include "NEAT.h"

namespace {
    int g_population_size = 150;
    //double g_select_rate = 
}

NEAT::NEAT_t::NEAT_t()
    :m_generation(0)//, m_population(g_population_size)
{ }

void NEAT::NEAT_t::nextGeneration()
{
    //NEATPopulation_t ngPopulation = m_population.selectBest();

    m_generation++;
}
