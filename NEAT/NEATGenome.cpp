#include "stdafx.h"
#include "NEATGenome.h"
#include <chrono>
#include <list>
#include <random>

int NEAT::ConnectionGene_t::g_nextInnovationNumber = 0;

//TODO: all of these constants are from the NEAT paper, credit it
const double NEAT::Genome_t::delta_c1 = 1.0;
const double NEAT::Genome_t::delta_c2 = 1.0;
const double NEAT::Genome_t::delta_c3 = 0.4;

const double NEAT::Genome_t::mutate_without_crossover_rate = 0.25;
const double NEAT::Genome_t::disable_gene_rate = 0.75;
const double NEAT::Genome_t::mutate_all_connection_weights = 0.8;
const double NEAT::Genome_t::generate_new_connection_weight = 0.1;
const double NEAT::Genome_t::mutate_add_connection = 0.05;
const double NEAT::Genome_t::mutate_add_node = 0.03;

namespace {
	double randDouble(double min = 0.0, double max = 1.0) {
		return (double)rand() / INT_MAX * (max - min) + min;
	}

	int randInt(int min = 0, int max = 100) {
		return rand() % (max - min + 1) + min;
	}

	std::map<std::pair<int, int>, NEAT::ConnectionGene_t> g_all_connections;

	// gaussian distribution, mean = 0.0, stdev = 1.0
	double getNormalizedRand(double mean = 0.0, double stdev = 1.0) {
		static unsigned seed((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
		static std::default_random_engine generator(seed);

		std::normal_distribution<double> distribution(mean, stdev);
		return distribution(generator);
	}
}

struct NEAT::Population_t::Impl {
	//std::map<int, Species_t> m_species;
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

//void NEAT::Population_t::addToCorrectSpecies(const Genome_t & genome)
//{
//	throw;
//}

//struct NEAT::Species_t::Impl {
//	std::map<int, NEAT::Genome_t> m_genomes;
//};
//
//NEAT::Species_t::Species_t() : pimpl(new Impl()) { }
//NEAT::Species_t::Species_t(const NEAT::Species_t & s) : pimpl(new Impl(*s.pimpl)) { }
//NEAT::Species_t & NEAT::Species_t::operator=(const NEAT::Species_t & s)
//{
//	if (this != &s)
//		pimpl = new Impl(*s.pimpl);
//
//	return *this;
//}
//NEAT::Species_t::~Species_t() { delete pimpl; }

struct NEAT::Genome_t::Impl {
	Impl() : m_next_node_gene(0), m_max_innovation_mutation(0), m_fitness(0.0), m_adjusted_fitness(0.0) { }

	bool checkIfRecurrentConnection(int innoNum);
	SparseMatrix_t<double> W_helper(int min_size, bool recurrent);

	// crossover/mutate helpers
	void mutateAllConnectionWeights();
	std::pair<int, int> findNewAddConnection();
	NEAT::ConnectionGene_t findConnectionAddNewNode();

	// don't use shared_ptr, we want our own copies to modify values
	std::map<int, NodeGene_t> m_node_genes;
	int m_next_node_gene;

	std::map<int, ConnectionGene_t> m_connection_genes;
	int m_max_innovation_mutation;

	double m_fitness;
	double m_adjusted_fitness;
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

void NEAT::Genome_t::Impl::mutateAllConnectionWeights()
{
	for (std::map<int, ConnectionGene_t>::iterator it = m_connection_genes.begin(); it != m_connection_genes.end(); ++it) {
		if (randDouble() < Genome_t::generate_new_connection_weight) {
			// SET to new normalized random number
			it->second.setWeight(getNormalizedRand());
		}
		else {
			// MODIFY by normalized random number
			it->second.setWeight(it->second.getWeight() + getNormalizedRand());
		}
	}
}

std::pair<int, int> NEAT::Genome_t::Impl::findNewAddConnection()
{
	// get all node keys
	std::vector<int> node_keys;
	for (std::map<int, NodeGene_t>::const_iterator it = m_node_genes.begin(); it != m_node_genes.end(); ++it) {
		node_keys.push_back(it->first);
	}
	int num_node_keys = (int)node_keys.size();

	if (m_connection_genes.size() == num_node_keys * num_node_keys) {
		// jeez... we're fully connected, that's crazy
		return std::make_pair(-1, -1);
	}
	else if (m_connection_genes.size() >= num_node_keys * num_node_keys * 0.9) {
		// this seems a bit labor intensive, but I'm quite worried about a highly connected neural network... would I be able to find that 1 in 
		// 10,000 missing connection? And I don't want to just abort in that event...
		std::set<std::pair<int, int>> connections_not_made_yet;
		for (int i = 0; i < num_node_keys; ++i) {
			for (int j = 0; j < num_node_keys; ++j) {
				connections_not_made_yet.insert(std::make_pair(node_keys[i], node_keys[j]));
			}
		}

		// from that list of all possible connections, now remove all legit connections
		for (std::map<int, ConnectionGene_t>::const_iterator it = m_connection_genes.begin(); it != m_connection_genes.end(); ++it) {
			std::pair<int, int> actual_connection(it->second.getInIndex(), it->second.getOutIndex());
			connections_not_made_yet.erase(actual_connection);
		}

		// NOW, we choose a random connection that we're guaranteed to create for the first time
		std::set<std::pair<int, int>>::const_iterator chosen_connection_it = connections_not_made_yet.begin();
		//TODO: O(n^2) for n = number of nodes...
		std::advance(chosen_connection_it, randInt(0, (int)connections_not_made_yet.size() - 1));

		return *chosen_connection_it;
	}
	else {
		// connectivity is low enough that I'm comfortable with guess-n-check
		int inNum, outNum;
		bool valid = false;
		do {
			inNum = node_keys[randInt(0, num_node_keys - 1)];
			outNum = node_keys[randInt(0, num_node_keys - 1)];

			std::map<std::pair<int, int>, ConnectionGene_t>::iterator all_it = g_all_connections.find(std::make_pair(inNum, outNum));
			if (all_it != g_all_connections.end()) {
				int innovationNum = all_it->second.getInnovationNumber();
				std::map<int, ConnectionGene_t>::iterator this_it = m_connection_genes.find(innovationNum);
				if (this_it != m_connection_genes.end()) {
					valid = true;
				}
			}
		} while (!valid);

		// hopefully that didn't take too long...
		return std::make_pair(inNum, outNum);
	}
}

NEAT::ConnectionGene_t NEAT::Genome_t::Impl::findConnectionAddNewNode()
{
	// select a random existing connection
	std::map<int, ConnectionGene_t>::iterator it = m_connection_genes.begin();
	std::advance(it, randInt(0, (int)m_connection_genes.size() - 1));
	it->second.disable();
	return it->second;
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

SparseMatrix_t<double> NEAT::Genome_t::Wxh(int min_size) const
{
	return pimpl->W_helper(min_size, false);
}

SparseMatrix_t<double> NEAT::Genome_t::Whh(int min_size) const
{
	return pimpl->W_helper(min_size, true);
}

double NEAT::Genome_t::getFitness() const
{
	return pimpl->m_fitness;
}

void NEAT::Genome_t::setFitness(double fitness)
{
	pimpl->m_fitness = fitness;
}

double NEAT::Genome_t::getAdjustedFitness() const
{
	return pimpl->m_adjusted_fitness;
}

void NEAT::Genome_t::setAdjustedFitness(double adjusted_fitness)
{
	pimpl->m_adjusted_fitness = adjusted_fitness;
}

NEAT::Genome_t NEAT::Genome_t::makeOffspring(const Genome_t & rhs) const
{
	Genome_t rval;

	if (randDouble() < Genome_t::mutate_without_crossover_rate)
		rval = *this;
	else {
		// perform crossover first
		rval = crossover(rhs);
	}

	rval.mutate();

	// these need to be recalculated, zero them out
	rval.setFitness(0.0);
	rval.setAdjustedFitness(0.0);

	return rval;
}

NEAT::Genome_t NEAT::Genome_t::crossover(const Genome_t & rhs) const
{
	// *this should always be the more fit parent
	if (rhs.getFitness() > getFitness())
		return rhs.crossover(*this);

	// this copies over all the nodes, matching genes, excess genes, and disjoint genes from the more fit parent! Wow!
	Genome_t rval(*this); 

	std::set<int> match, disjoint_lhs, disjoint_rhs, excess_lhs, excess_rhs;
	rval.calculateDisjointExcess(rhs, match, disjoint_lhs, disjoint_rhs, excess_lhs, excess_rhs);

	for (std::map<int, ConnectionGene_t>::iterator lhs_it = rval.pimpl->m_connection_genes.begin(); lhs_it != rval.pimpl->m_connection_genes.end(); ++lhs_it) {
		if (match.find(lhs_it->first) != match.end()) {
			// matching gene, select weight randomly
			std::map<int, ConnectionGene_t>::const_iterator rhs_it = rhs.pimpl->m_connection_genes.find(lhs_it->first);

			if (randInt(0, 1) == 1) // flip a coin
				lhs_it->second.setWeight(rhs_it->second.getWeight());
			// else keep the lhs's weight

			// if EITHER were disabled, chance the child will be disabled too
			if (!lhs_it->second.isEnabled() || !rhs_it->second.isEnabled()) {
				if (randDouble() < Genome_t::disable_gene_rate) {
					lhs_it->second.disable();
				}
				else {
					lhs_it->second.enable();
				}
			}
		}
		else {
			// either disjoint_lhs or excess_lhs
			// weight is fine as is
			// if this parent was disabled, chance the child will be disabled too
			if (!lhs_it->second.isEnabled()) {
				if (randDouble() < Genome_t::disable_gene_rate) {
					lhs_it->second.disable();
				}
				else {
					lhs_it->second.enable();
				}
			}
		}
	}

	// values crossed over, disjoint & excess all from parent. We're done!
	return rval;
}

void NEAT::Genome_t::mutate()
{
	// should we modify the connection weights?
	if (randDouble() < Genome_t::mutate_all_connection_weights) {
		pimpl->mutateAllConnectionWeights();
	}

	// should we add a brand new connection?
	if (randDouble() < Genome_t::mutate_add_connection) {
		std::pair<int, int> newConnectionPair = pimpl->findNewAddConnection();
		addConnectionGene(newConnectionPair.first, newConnectionPair.second, getNormalizedRand());
	}

	// should we add a brand new node?
	if (randDouble() < Genome_t::mutate_add_node) {
		int newNodeIndex = addNodeGene(NEAT::HIDDEN_NODE, "Hidden");
		if (newNodeIndex != -1) { // no errors
			NEAT::ConnectionGene_t oldConnectionCopy = pimpl->findConnectionAddNewNode();
			addConnectionGene(oldConnectionCopy.getInIndex(), newNodeIndex, 1.0);
			addConnectionGene(newNodeIndex, oldConnectionCopy.getOutIndex(), oldConnectionCopy.getWeight());
		}
	}
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

//TODO: Since the genome handles values and hidden nodes and such, i might not need this at all
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
