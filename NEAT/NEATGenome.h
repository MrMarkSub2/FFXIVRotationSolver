#pragma once

#ifdef NEAT_EXPORTS
#define NEAT_API __declspec(dllexport)
#else
#define NEAT_API __declspec(dllimport)
#endif

#include <map>
#include <set>
#include "SparseMatrix.h"

namespace NEAT {
	class NEAT_API ConnectionGene_t {
	public:
		ConnectionGene_t(int in, int out, double weight);
		ConnectionGene_t(const ConnectionGene_t& c);

		int getInIndex() const { return m_in_index; }
		int getOutIndex() const { return m_out_index; }
		double getWeight() const { return m_weight; }
		bool isEnabled() const { return m_enabled; }
		int getInnovationNumber() const { return m_innovation_number; }
		bool isRecurrent() const { return m_recurrent; }

		void setWeight(double weight) { m_weight = weight; }
		void enable() { m_enabled = true; }
		void disable() { m_enabled = false; }
		void setRecurrent(bool recurrent) { m_recurrent = recurrent; }

		bool isEquivalentConnection(const ConnectionGene_t& c) const;
		bool isEquivalentConnection(int in, int out) const;

	private:
		int m_in_index;
		int m_out_index;
		double m_weight;
		bool m_enabled;
		int m_innovation_number;
		bool m_recurrent;

		static int g_nextInnovationNumber;
	};

	enum NODETYPE {
		INPUT_NODE,
		OUTPUT_NODE,
		HIDDEN_NODE
	};

	class NEAT_API NodeGene_t {
	public:
		NodeGene_t(int index, NODETYPE nodetype, const std::string& label);
		NodeGene_t(const NodeGene_t & ng);
		NodeGene_t& NodeGene_t::operator=(const NodeGene_t& ng);
		~NodeGene_t();

		std::string getLabel() const;

	private:
		struct Impl;
		Impl* pimpl;
	};

	class NEAT_API Genome_t {
	public:
		Genome_t();
		Genome_t(const Genome_t & g);
		Genome_t& Genome_t::operator=(const Genome_t& g);
		~Genome_t();

		int addNodeGene(NODETYPE nodetype, const std::string& label); // returns -1 if already exists
		int addConnectionGene(int in, int out, double weight); // returns -1 if already exists
		bool updateConnectionGene(int innovation, double weight); // returns false if does not exist
		bool updateConnectionGene(int in, int out, double weight); // returns false if does not exist

		double delta(const Genome_t& rhs) const;
		void calculateDisjointExcess(const Genome_t& rhs, std::set<int>& match, std::set<int>& disjoint_lhs, std::set<int>& disjoint_rhs,
		                             std::set<int>& excess_lhs, std::set<int>& excess_rhs) const;

		// these matrices are used for fitness evaluation via matrix-vector multiplication
		//TODO: I think I'm doing this wrong.
		/* http://www.cs.ucf.edu/~kstanley/neat.html
		 * The activation function, bool Network::activate(), gives the specifics. The implementation is of course considerably different than for a 
		 * simple layered feedforward network. Each node adds up the activation from all incoming nodes from the previous timestep. (The function also 
		 * handles a special "time delayed" connection, but that is not used by the current version of NEAT in any experiments that we have published.) 
		 * Another way to understand it is to realize that activation does not travel all the way from the input layer to the output layer in a single 
		 * timestep. In a single timestep, activation only travels from one neuron to the next. So it takes several timesteps for activation to get from 
		 * the inputs to the outputs. If you think about it, this is the way it works in a real brain, where it takes time for a signal hitting your eyes 
		 * to get to the cortex because it travels over several neural connections. 
		 */
		//TODO: I saw the delay in traveling through multiple layers in a single step, and assumed bug. This insinuates feature.
		SparseMatrix_t<double> Wxh(int min_size = 0) const; // input to hidden, all non-recurrent connections
		SparseMatrix_t<double> Whh(int min_size = 0) const; // hidden to hidden, all recurrent connections

		// calculating fitness depends on the model which lives in an unlinked toolkit. We rely on the mainprog to set this for us
		double getFitness() const;
		void setFitness(double fitness);

		// adjusted value dependent on sigma delta, used in determining speciation
		double getAdjustedFitness() const;
		void setAdjustedFitness(double adjusted_fitness);

		Genome_t makeOffspring(const Genome_t& rhs) const;

	private:
		struct Impl;
		Impl *pimpl;

		Genome_t crossover(const Genome_t& rhs) const;
		void mutate();

		static const double delta_c1;
		static const double delta_c2;
		static const double delta_c3;

		static const double mutate_without_crossover_rate;
		static const double disable_gene_rate;
		static const double mutate_all_connection_weights;
		static const double generate_new_connection_weight;
		static const double mutate_add_connection;
		static const double mutate_add_node;
	};

	class NEAT_API Population_t {
	public:
		Population_t();
		Population_t(const Population_t & p);
		Population_t& Population_t::operator=(const Population_t& p);
		~Population_t();

		// genome ids are always 0 to n contiguously, as opposed to connection/species innovation number
		int size() const;

		int getGeneration() const;

		// const reference, don't let us modify directly through this
		const Genome_t& getGenome(int id) const;
		// since we can't modify a genome through the getter, but we do need access to setFitness, we expose it here
		// added benefit: this helps us track the species's champion
		void setFitness(int id, double fitness);

		// returns list of all species (e.g. species #9, species #14, species #20)
		std::vector<int> getSpeciesIds() const;
		// returns list of all genomes in a given species (e.g. species #9 contains genomes #26, #87, #186)
		std::vector<int> getGenomeIdsOfSpecies(int speciesId) const;
		// returns list of just the top percentile of genomes in a given species
		std::vector<int> getBestGenomeIdsOfSpecies(int speciesId, double percentile) const;
		// which genome is the fittest of a given species
		int getFittestGenomeIdofSpecies(int speciesId) const;
		// which genome is the fittest of this generation, period
		int getFittestGenomeId() const;

		// generally more appropriate, finds if your genome is close enough to an existing species or if it's its own thing
		// returns new genome's id
		int addToCorrectSpecies(const Genome_t& genome, std::map<int, Genome_t>& representatives);
		// useful for bringing a species from last generation over and keeping its old speciesId
		// returns new genome's id
		int addToSpecificSpecies(const Genome_t& genome, int speciesId);

		//TODO: If the maximum fitness of a species did not improve in 15 generations, the networks in the stagnant species were not allowed to reproduce
		//      Do I want this? It seems like this could eliminate strong contenders

		// of course, one of the most important functions: evolve us to the next generation!
		// please refrain from making TNG jokes...
		Population_t createNextGeneration();

		static const int genome_count;
		
	private:
		struct Impl;
		Impl* pimpl;

		static const double starting_delta_t;
		static const double delta_t_step;
		static const int target_species_count;
		static const double breeding_percentile;
		static const int min_species_size;
		static const int species_stagnation_limit;
		static const double interspecies_mating_rate;

		static int g_nextSpeciesNumber;
	};
}
