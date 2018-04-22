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
		double getValue() const;

		void setValue(double val);

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
		SparseMatrix_t<double> Wxh(int min_size = 0) const; // input to hidden, all non-recurrent connections
		SparseMatrix_t<double> Whh(int min_size = 0) const; // hidden to hidden, all recurrent connections

		// calculating fitness depends on the model which lives in an unlinked toolkit. We rely on the mainprog to set this for us
		double getFitness() const;
		void setFitness(double fitness);

		// adjusted value dependent on sigma delta, used in determining
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

	//class NEAT_API Species_t {
	//public:
	//	Species_t();
	//	Species_t(const Species_t & s);
	//	Species_t& Species_t::operator=(const Species_t& s);
	//	~Species_t();

	//	std::vector<int> getGenomeIds() const;
	//	// const reference, don't let us modify directly through this
	//	Genome_t& getGenome(int id) const;
	//	// since we can't modify a genome through the getter, but we do need access to setFitness, we expose it here
	//	// added benefit: this helps us track the species's champion
	//	void setFitness(int id, double fitness);

	//private:
	//	struct Impl;
	//	Impl* pimpl;	
	//};

	class NEAT_API Population_t {
	public:
		Population_t();
		Population_t(const Population_t & p);
		Population_t& Population_t::operator=(const Population_t& p);
		~Population_t();

		//TODO: might make some of these functions private

		//// genome ids are always 0 to n contiguously, as opposed to connection/species innovation number
		//int size() const;

		//// const reference, don't let us modify directly through this
		//Genome_t& getGenome(int id) const;
		//// since we can't modify a genome through the getter, but we do need access to setFitness, we expose it here
		//// added benefit: this helps us track the species's champion
		//void setFitness(int id, double fitness);
		//void setAdjustedFitness(int id, double adjusted_fitness);

		//// returns list of all species (e.g. species #9, species #14, species #20)
		//std::vector<int> getSpeciesIds() const;
		//// returns list of all genomes in a given species (e.g. species #9 contains genomes #26, #87, #186)
		//std::vector<int> getGenomeIdsOfSpecies(int speciesId) const;
		//// which genome is the fittest of a given species
		//int getFittestGenomeIdofSpecies(int speciesId) const;

		//// generally more appropriate, finds if your genome is close enough to an existing species or if it's its own thing
		//void addToCorrectSpecies(const Genome_t& genome);
		//// useful for bringing a species from last generation over and keeping its old speciesId
		//void addToSpecificSpecies(const Genome_t& genome, int speciesId);
		
	private:
		struct Impl;
		Impl* pimpl;
	};
}
