#pragma once

#ifdef NEST_EXPORTS
#define NEST_API __declspec(dllexport)
#else
#define NEST_API __declspec(dllimport)
#endif

class NEST_API NEST_t {
public:
    NEST_t();
    virtual ~NEST_t() = 0;

    void nextGeneration();


protected:
    //virtual NESTPopulation_t InitializePopulation() = 0;

private:
    NESTPopulation_t m_population;
    int m_generation;
};