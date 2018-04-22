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
		SparseMatrix_t<double> Wxh(int min_size = 0); // input to hidden, all non-recurrent connections
		SparseMatrix_t<double> Whh(int min_size = 0); // hidden to hidden, all recurrent connections

	private:
		struct Impl;
		Impl *pimpl;

		static const double delta_c1;
		static const double delta_c2;
		static const double delta_c3;
	};

	class NEAT_API Species_t {
	public:
		Species_t();
		Species_t(const Species_t & s);
		Species_t& Species_t::operator=(const Species_t& s);
		~Species_t();

	private:
		struct Impl;
		Impl* pimpl;	
	};

	class NEAT_API Population_t {
	public:
		Population_t();
		Population_t(const Population_t & p);
		Population_t& Population_t::operator=(const Population_t& p);
		~Population_t();

		void addToCorrectSpecies(const Genome_t& genome);
	private:
		struct Impl;
		Impl* pimpl;
	};
}
