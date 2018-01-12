#pragma once

#ifdef NEAT_EXPORTS
#define NEAT_API __declspec(dllexport)
#else
#define NEAT_API __declspec(dllimport)
#endif

namespace NEAT {
	class NEAT_API NEAT_t {
	public:
		NEAT_t();
		//virtual ~NEAT_t() = 0;

		void nextGeneration();


	protected:
		//virtual NEATPopulation_t InitializePopulation() = 0;

	private:
		//NEATPopulation_t m_population;
		int m_generation;
	};
}
