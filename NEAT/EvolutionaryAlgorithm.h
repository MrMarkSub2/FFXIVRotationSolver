#pragma once

#include "rdm_moves.h"

#ifdef NEAT_EXPORTS
#define NEAT_API __declspec(dllexport)
#else
#define NEAT_API __declspec(dllimport)
#endif

/*class NEAT_API EvolutionaryAlgorithm_t {
public:
    EvolutionaryAlgorithm_t();
    virtual ~EvolutionaryAlgorithm_t() = 0;

    void nextGeneration();


protected:
    virtual Population_t* InitializePopulation() = 0;
    
private:
    Population_t* m_population;
    int m_generation;
};*/

/*class NEAT_API TestAlgorithm_t : public EvolutionaryAlgorithm_t {

};*/