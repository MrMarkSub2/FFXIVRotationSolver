#pragma once

#ifdef NEAT_EXPORTS
#define NEAT_API __declspec(dllexport)
#else
#define NEAT_API __declspec(dllimport)
#endif

#include <map>
#include <memory>
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

		void setWeight(double weight);
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
		const static double weight_cap;
	};

	enum NODETYPE {
		INPUT_NODE,
		OUTPUT_NODE,
		HIDDEN_NODE
	};

	class NodeGene_t {
	public:
		NEAT_API NodeGene_t(int index, NODETYPE nodetype, const std::string& label);
		NEAT_API NodeGene_t(NodeGene_t && ng) noexcept;
		NEAT_API NodeGene_t& NodeGene_t::operator=(NodeGene_t && ng) noexcept;
		NEAT_API NodeGene_t(const NodeGene_t & ng);
		NEAT_API NodeGene_t& NodeGene_t::operator=(const NodeGene_t& ng);
		NEAT_API ~NodeGene_t();

		std::string NEAT_API getLabel() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl;
	};

	class Genome_t {
	public:
		NEAT_API Genome_t();
		NEAT_API Genome_t(Genome_t && g) noexcept;
		NEAT_API Genome_t& operator=(Genome_t && g) noexcept;
		NEAT_API Genome_t(const Genome_t & g);
		NEAT_API Genome_t& Genome_t::operator=(const Genome_t& g);
		NEAT_API ~Genome_t();

		int NEAT_API addNodeGene(NODETYPE nodetype, const std::string& label); // returns -1 if already exists
		int NEAT_API addConnectionGene(int in, int out, double weight); // returns -1 if already exists
		bool NEAT_API updateConnectionGene(int innovation, double weight); // returns false if does not exist
		bool NEAT_API updateConnectionGene(int in, int out, double weight); // returns false if does not exist

		double NEAT_API delta(const Genome_t& rhs) const;
		void NEAT_API calculateDisjointExcess(const Genome_t& rhs, std::set<int>& match, std::set<int>& disjoint_lhs, std::set<int>& disjoint_rhs,
		                             std::set<int>& excess_lhs, std::set<int>& excess_rhs) const;

		// these matrices are used for fitness evaluation via matrix-vector multiplication
		SparseMatrix_t<double> NEAT_API Wxh(int min_size = 0) const;
		SparseMatrix_t<double> NEAT_API Whh(int min_size = 0) const;

		// calculating fitness depends on the model which lives in an unlinked toolkit. We rely on the mainprog to set this for us
		double NEAT_API getFitness() const;
		void NEAT_API setFitness(double fitness);

		Genome_t NEAT_API makeOffspring(const Genome_t& rhs) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl;

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

	class Population_t {
	public:
		NEAT_API Population_t();
		NEAT_API Population_t(Population_t && p) noexcept;
		NEAT_API Population_t& Population_t::operator=(Population_t && p) noexcept;
		NEAT_API Population_t(const Population_t & p);
		NEAT_API Population_t& Population_t::operator=(const Population_t& p);
		NEAT_API ~Population_t();

		// genome ids are always 0 to n contiguously, as opposed to connection/species innovation number
		int NEAT_API size() const;

		int NEAT_API getGeneration() const;

		// const reference, don't let us modify directly through this
		const NEAT::Genome_t NEAT_API & getGenome(int id) const;
		// since we can't modify a genome through the getter, but we do need access to setFitness, we expose it here
		// added benefit: this helps us track the species's champion
		void NEAT_API setFitness(int id, double fitness);

		// returns list of all species (e.g. species #9, species #14, species #20)
		std::vector<int> NEAT_API getSpeciesIds() const;
		// returns list of all genomes in a given species (e.g. species #9 contains genomes #26, #87, #186)
		std::vector<int> NEAT_API getGenomeIdsOfSpecies(int speciesId) const;
		// returns list of just the top percentile of genomes in a given species
		std::vector<int> NEAT_API getBestGenomeIdsOfSpecies(int speciesId, double percentile) const;
		// which genome is the fittest of a given species
		int NEAT_API getFittestGenomeIdofSpecies(int speciesId) const;
		// which genome is the fittest of this generation, period
		int NEAT_API getFittestGenomeId() const;

		// generally more appropriate, finds if your genome is close enough to an existing species or if it's its own thing
		// returns new genome's id
		int NEAT_API addToCorrectSpecies(const Genome_t& genome, std::map<int, Genome_t>& representatives);
		// useful for bringing a species from last generation over and keeping its old speciesId
		// returns new genome's id
		int NEAT_API addToSpecificSpecies(const Genome_t& genome, int speciesId);

		// of course, one of the most important functions: evolve us to the next generation!
		// please refrain from making TNG jokes...
		Population_t NEAT_API createNextGeneration();

		static const int NEAT_API genome_count;
		
	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl;

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
