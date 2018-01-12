#pragma once

#include "moves.h"

#ifdef NEST_EXPORTS
#define NEST_API __declspec(dllexport)
#else
#define NEST_API __declspec(dllimport)
#endif

/*class NEST_API EvolutionaryAlgorithm_t {
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

/*class NEST_API TestAlgorithm_t : public EvolutionaryAlgorithm_t {

};*/