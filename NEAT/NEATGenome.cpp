#include "stdafx.h"
#include "NEATGenome.h"
#include <list>

int NEAT::ConnectionGene_t::g_nextInnovationNumber = 0;

const double NEAT::Genome_t::delta_c1 = 1.0;
const double NEAT::Genome_t::delta_c2 = 1.0;
const double NEAT::Genome_t::delta_c3 = 0.4;

namespace {
	double randDouble(double min = 0.0, double max = 1.0) {
		return (double)rand() / INT_MAX * (max - min) + min;
	}

	int randInt(int min = 0, int max = 100) {
		return rand() % (max - min + 1) + min;
	}

	std::map<std::pair<int, int>, NEAT::ConnectionGene_t> g_all_connections;
}

struct NEAT::Population_t::Impl {
	std::map<int, Species_t> m_species;
};

NEAT::Population_t::Population_t() : pimpl(new Impl()) { }
NEAT::Population_t::Population_t(const Population_t & p) : pimpl(new Impl(*p.pimpl)) { }
NEAT::Population_t & NEAT::Population_t::operator=(const NEAT::Population_t & p)
{
	if (this != &p)
		pimpl = new Impl(*p.pimpl);

	return *this;
}
NEAT::Population_t::~Population_t() { delete pimpl; }

void NEAT::Population_t::addToCorrectSpecies(const Genome_t & genome)
{
	throw;
}

struct NEAT::Species_t::Impl {
	std::map<int, NEAT::Genome_t> m_genomes;
};

NEAT::Species_t::Species_t() : pimpl(new Impl()) { }
NEAT::Species_t::Species_t(const NEAT::Species_t & s) : pimpl(new Impl(*s.pimpl)) { }
NEAT::Species_t & NEAT::Species_t::operator=(const NEAT::Species_t & s)
{
	if (this != &s)
		pimpl = new Impl(*s.pimpl);

	return *this;
}
NEAT::Species_t::~Species_t() { delete pimpl; }

struct NEAT::Genome_t::Impl {
	Impl() : m_next_node_gene(0), m_max_innovation_mutation(0) {}

	bool checkIfRecurrentConnection(int innoNum);
	SparseMatrix_t<double> W_helper(int min_size, bool recurrent);

	// don't use shared_ptr, we want our own copies to modify values
	std::map<int, NodeGene_t> m_node_genes;
	int m_next_node_gene;

	std::map<int, ConnectionGene_t> m_connection_genes;
	int m_max_innovation_mutation;
};

bool NEAT::Genome_t::Impl::checkIfRecurrentConnection(int innoNum)
{
	std::list<int> connectionBFS;
	std::map<int, ConnectionGene_t>::iterator testConnection_it = m_connection_genes.find(innoNum);
	connectionBFS.push_back(testConnection_it->second.getOutIndex());
	const int inIndex = testConnection_it->second.getInIndex();

	// can we get from outIndex to inIndex using only non-recurrent edges? If so, then adding this edge will create a recurrent loop
	while (!connectionBFS.empty()) {
		const int currentNode = connectionBFS.front();
		connectionBFS.pop_front();

		for (std::map<int, ConnectionGene_t>::const_iterator it = m_connection_genes.begin(); it != m_connection_genes.end(); ++it) {
			if (it->second.isEnabled() && !it->second.isRecurrent()) {
				if (it->second.getInIndex() == currentNode) {
					if (it->second.getOutIndex() == inIndex) {
						// we found a cycle!
						testConnection_it->second.setRecurrent(true);
						return true;
					}
					else {
						connectionBFS.push_back(it->second.getOutIndex());
					}
				}
			}
		}
	}

	return false;
}

SparseMatrix_t<double> NEAT::Genome_t::Impl::W_helper(int min_size, bool recurrent)
{
	const int size = max(min_size, m_next_node_gene);
	SparseMatrix_t<double> rval(size, size);

	for (std::map<int, ConnectionGene_t>::const_iterator it = m_connection_genes.begin(); it != m_connection_genes.end(); ++it) {
		if (it->second.isEnabled() && it->second.isRecurrent() == recurrent) {
			// yes, out is the row, in is the column. Since the state is a vertical vector, imagine input[0] = 1, and we have a connection (0, 4).
			// In that example we would expect node 4 to be lit in the resultant vector
			rval.addElem(it->second.getWeight(), it->second.getOutIndex(), it->second.getInIndex());
		}
	}

	return rval.fullyExpand();
}

NEAT::Genome_t::Genome_t()
	: pimpl(new Impl())
{
	// generate all standardized node genes
	// use enum values so that I can always figure out what node maps to what
}
NEAT::Genome_t::Genome_t(const NEAT::Genome_t & g) : pimpl(new Impl(*g.pimpl)) { }
NEAT::Genome_t & NEAT::Genome_t::operator=(const NEAT::Genome_t & g)
{
	if (this != &g)
		pimpl = new Impl(*g.pimpl);

	return *this;
}
NEAT::Genome_t::~Genome_t() { delete pimpl; }

int NEAT::Genome_t::addNodeGene(NODETYPE nodetype, const std::string & label)
{
	std::pair<std::map<int, NodeGene_t>::iterator, bool> add_rval = pimpl->m_node_genes.insert(std::make_pair(pimpl->m_next_node_gene, NodeGene_t(pimpl->m_next_node_gene, nodetype, label)));
	if (add_rval.second)
		return pimpl->m_next_node_gene++;
	else
		return -1;
}

int NEAT::Genome_t::addConnectionGene(int in, int out, double weight)
{
	// first check if the connection gene exists anywhere
	std::map<std::pair<int, int>, ConnectionGene_t>::iterator all_it = g_all_connections.find(std::make_pair(in, out));
	if (all_it != g_all_connections.end()) {
		ConnectionGene_t existingGene(all_it->second);
		existingGene.setWeight(weight);
		std::pair<std::map<int, ConnectionGene_t>::iterator, bool> add_rval = pimpl->m_connection_genes.insert(std::make_pair(existingGene.getInnovationNumber(), existingGene));
		if (add_rval.second) {
			pimpl->checkIfRecurrentConnection(existingGene.getInnovationNumber());
			return existingGene.getInnovationNumber();
		}
		else
			return -1;
	}
	
	// not found, let's add
	ConnectionGene_t newGene(in, out, weight); // increments global innovation number
	pimpl->m_connection_genes.insert(std::make_pair(newGene.getInnovationNumber(), newGene));
	pimpl->m_max_innovation_mutation = newGene.getInnovationNumber();
	g_all_connections.insert(std::make_pair(std::make_pair(in, out), newGene));
	pimpl->checkIfRecurrentConnection(newGene.getInnovationNumber());
	return newGene.getInnovationNumber();
}

bool NEAT::Genome_t::updateConnectionGene(int innovation, double weight)
{
	// check if the connection gene is already established in this genome
	std::map<int, ConnectionGene_t>::iterator this_it = pimpl->m_connection_genes.find(innovation);
	if (this_it != pimpl->m_connection_genes.end()) {
		this_it->second.setWeight(weight);
		return true;
	}

	return false;
}

bool NEAT::Genome_t::updateConnectionGene(int in, int out, double weight)
{
	std::map<std::pair<int, int>, ConnectionGene_t>::iterator all_it = g_all_connections.find(std::make_pair(in, out));
	if (all_it != g_all_connections.end())
		return updateConnectionGene(all_it->second.getInnovationNumber(), weight);
	else
		return false;
}

double NEAT::Genome_t::delta(const Genome_t & rhs) const
{
	std::set<int> match, disjoint_lhs, disjoint_rhs, excess_lhs, excess_rhs;
	calculateDisjointExcess(rhs, match, disjoint_lhs, disjoint_rhs, excess_lhs, excess_rhs);

	const size_t excess_cnt = excess_lhs.size() + excess_rhs.size();
	const size_t disjoint_cnt = disjoint_lhs.size() + disjoint_rhs.size();
	const size_t num = match.size() + max(disjoint_lhs.size() + excess_lhs.size(), disjoint_rhs.size() + excess_rhs.size());

	double weight_diff(0.0);
	for (std::set<int>::const_iterator it = match.begin(); it != match.end(); ++it) {
		weight_diff += abs(pimpl->m_connection_genes.find(*it)->second.getWeight() - rhs.pimpl->m_connection_genes.find(*it)->second.getWeight());
	}

	return delta_c1 * excess_cnt / num + delta_c2 * disjoint_cnt / num + delta_c3 * weight_diff / match.size();
}

void NEAT::Genome_t::calculateDisjointExcess(const Genome_t & rhs, std::set<int>& match, std::set<int>& disjoint_lhs, std::set<int>& disjoint_rhs,
	                                         std::set<int>& excess_lhs, std::set<int>& excess_rhs) const
{
	std::map<int, ConnectionGene_t>::const_iterator lhs_it = pimpl->m_connection_genes.begin();
	std::map<int, ConnectionGene_t>::const_iterator rhs_it = rhs.pimpl->m_connection_genes.begin();

	while (lhs_it != pimpl->m_connection_genes.end() && rhs_it != rhs.pimpl->m_connection_genes.end()) {
		if (lhs_it->first == rhs_it->first) {
			match.insert(lhs_it->first);
			++lhs_it;
			++rhs_it;
		} 
		else if (lhs_it->first < rhs_it->first) { // missing from rhs
			disjoint_lhs.insert(lhs_it->first);
			++lhs_it;
		}
		else { // missing from lhs
			disjoint_rhs.insert(rhs_it->first);
			++rhs_it;
		}
	}

	// one or the other is finished; add the rest to excess
	while (lhs_it != pimpl->m_connection_genes.end()) {
		excess_lhs.insert(lhs_it->first);
		++lhs_it;
	}

	while (rhs_it != rhs.pimpl->m_connection_genes.end()) {
		excess_rhs.insert(rhs_it->first);
		++rhs_it;
	}
}

SparseMatrix_t<double> NEAT::Genome_t::Wxh(int min_size)
{
	return pimpl->W_helper(min_size, false);
}

SparseMatrix_t<double> NEAT::Genome_t::Whh(int min_size)
{
	return pimpl->W_helper(min_size, true);
}

/*NEAT::Genome_t NEAT::Genome_t::createMutation()
{
	Genome_t newGenome(*this);
	for (std::map<int, NEAT::ConnectionGene_t>::iterator it = newGenome.pimpl->m_connection_genes.begin();
		it != newGenome.pimpl->m_connection_genes.end(); ++it) {
		// mutating connection weights
		if (randPercent() < Genome_t::g_connection_weight_mutation) {
			if (randPercent() < Genome_t::g_connection_weight_uniform) {
				// modify
				double weight = it->second.getWeight();
				it->second.setWeight(weight * (randPercent() * 4.0 - 2.0));
			}
			else {
				// randomize
				it->second.setWeight(randPercent() * 2.0 - 1.0);
			}
		}

		if (randPercent() < Genome_t::g_connection_enabled)
	}
	
}*/

struct NEAT::NodeGene_t::Impl {
	Impl(int index, NODETYPE nodetype, const std::string& label);

	int m_index;
	NODETYPE m_nodetype;
	std::string m_label;
	double m_value;
};

NEAT::NodeGene_t::Impl::Impl(int index, NODETYPE nodetype, const std::string& label)
	: m_index(index), m_nodetype(nodetype), m_label(label), m_value(0.0)
{
}

NEAT::NodeGene_t::NodeGene_t(int index, NODETYPE nodetype, const std::string & label) : pimpl(new Impl(index, nodetype, label)) { }
NEAT::NodeGene_t::NodeGene_t(const NEAT::NodeGene_t & ng) : pimpl(new Impl(*ng.pimpl)) { }
NEAT::NodeGene_t & NEAT::NodeGene_t::operator=(const NEAT::NodeGene_t & ng)
{
	if (this != &ng)
		pimpl = new Impl(*ng.pimpl);

	return *this;
}
NEAT::NodeGene_t::~NodeGene_t() { delete pimpl; }

std::string NEAT::NodeGene_t::getLabel() const
{
	return pimpl->m_label;
}

double NEAT::NodeGene_t::getValue() const
{
	return pimpl->m_value;
}

void NEAT::NodeGene_t::setValue(double val)
{
	pimpl->m_value = val;
}

NEAT::ConnectionGene_t::ConnectionGene_t(int in, int out, double weight)
	: m_in_index(in), m_out_index(out), m_weight(weight), m_enabled(true)
{
	//reserve this innovation number, and increment to the next
	m_innovation_number = g_nextInnovationNumber++;
}

NEAT::ConnectionGene_t::ConnectionGene_t(const ConnectionGene_t & c)
	: m_in_index(c.m_in_index), 
	  m_out_index(c.m_out_index),
	  m_weight(c.m_weight), 
	  m_enabled(c.m_enabled),
	  m_innovation_number(c.m_innovation_number),
	  m_recurrent(false)
{
	// we transfer the innovation number. This is a true copy, not a clone
}

bool NEAT::ConnectionGene_t::isEquivalentConnection(const ConnectionGene_t & c) const
{
	return isEquivalentConnection(c.getInIndex(), c.getOutIndex());
}

bool NEAT::ConnectionGene_t::isEquivalentConnection(int in, int out) const
{
	return (m_in_index == in) && (m_out_index == out);
}
