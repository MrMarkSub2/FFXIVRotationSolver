#include "stdafx.h"
#include "NEATGenome.h"
#include <iterator>
#include <list>

int NEAT::ConnectionGene_t::g_nextInnovationNumber = 0;
int NEAT::Population_t::g_nextSpeciesNumber = 0;

// all of these constants are from the NEAT paper and/or FAQ
const double NEAT::Genome_t::delta_c1 = 2.0;
const double NEAT::Genome_t::delta_c2 = 2.0;
const double NEAT::Genome_t::delta_c3 = 1.0;

const double NEAT::Genome_t::mutate_without_crossover_rate = 0.25;
const double NEAT::Genome_t::disable_gene_rate = 0.75;
const double NEAT::Genome_t::mutate_all_connection_weights = 0.8;
const double NEAT::Genome_t::generate_new_connection_weight = 0.1;
const double NEAT::Genome_t::mutate_add_connection = 0.05; // 0.3;
const double NEAT::Genome_t::mutate_add_node = 0.03; //0.05

const double NEAT::Population_t::starting_delta_t = 6.0;
const double NEAT::Population_t::delta_t_step = 0.3;// 0.3;
const int NEAT::Population_t::genome_count = 150; // 500; //150 // seems small for my complexity, consider 200-500... depends on how fast the fitness function is
const int NEAT::Population_t::target_species_count = 15; //50; //15 // probably needs experimentation
const double NEAT::Population_t::breeding_percentile = 0.2; // weaker genomes don't get to reproduce. Percentile applied to each species separately
const int NEAT::Population_t::species_stagnation_limit = 15; // start to choke it off, it's an evolutionary dead-end
const int NEAT::Population_t::min_species_size = 5; // don't let us have species with just 1 or 2 genomes
const double NEAT::Population_t::interspecies_mating_rate = 0.001; // very tiny chance of breeding between two different species

namespace {
	std::map<std::pair<int, int>, NEAT::ConnectionGene_t> g_all_connections;
}

struct NEAT::Population_t::Impl {
	Impl();

	double calculateAdjustedFitness(int genome, int species);
	void registerHighestFitness(int species, double fitness);

	struct Species_t {
		Species_t()
			: m_highest_fitness(0.0), m_stale_generations(0)
		{ }

		double staleSpeciesPenalty() const;

		std::set<int> m_genomes;
		double m_highest_fitness;
		int m_stale_generations;
	};

	int m_generation_num;
	std::vector<NEAT::Genome_t> m_genomes;
	std::map<int, Species_t> m_species_lists;
	double delta_t;
};

NEAT::Population_t::Impl::Impl()
	: m_generation_num(0), delta_t(starting_delta_t)
{ }

double NEAT::Population_t::Impl::calculateAdjustedFitness(int genome, int species)
{
	int denom = 1;
	double penalty = 1.0;
	std::map<int, Species_t>::const_iterator it = m_species_lists.find(species);
	if (it != m_species_lists.end()) {
		denom = (int)it->second.m_genomes.size();
		penalty = it->second.staleSpeciesPenalty();
	}
	
	return m_genomes[genome].getFitness() / denom * penalty;
}

void NEAT::Population_t::Impl::registerHighestFitness(int species, double fitness)
{
	std::map<int, Species_t>::iterator it = m_species_lists.find(species);
	if (it != m_species_lists.end()) {
		if (fitness > it->second.m_highest_fitness) {
			it->second.m_highest_fitness = fitness;
			it->second.m_stale_generations = 0;
		}
		else {
			++it->second.m_stale_generations;
		}
	}
}

double NEAT::Population_t::Impl::Species_t::staleSpeciesPenalty() const
{
	if (m_stale_generations <= NEAT::Population_t::species_stagnation_limit)
		return 1.0; // no penalty!

	return (double)NEAT::Population_t::species_stagnation_limit / m_stale_generations; // slowly degrade score of dead-end species
}

NEAT::Population_t::Population_t() : pimpl(new Impl()) { }
NEAT::Population_t::Population_t(NEAT::Population_t && p) noexcept = default;
NEAT::Population_t& NEAT::Population_t::operator=(NEAT::Population_t && p) noexcept = default;
NEAT::Population_t::Population_t(const NEAT::Population_t & p) : pimpl(new Impl(*p.pimpl)) { }
NEAT::Population_t & NEAT::Population_t::operator=(const NEAT::Population_t & p)
{
	if (this != &p)
		pimpl.reset(new Impl(*p.pimpl));

	return *this;
}
NEAT::Population_t::~Population_t() = default;

int NEAT::Population_t::size() const
{
	return (int)pimpl->m_genomes.size();
}

int NEAT::Population_t::getGeneration() const
{
	return pimpl->m_generation_num;
}

const NEAT::Genome_t & NEAT::Population_t::getGenome(int id) const
{
	if (id < 0 || id >= size())
		throw std::runtime_error(std::string("Invalid genome id passed to NEAT::Population_t::getGenome"));

	return pimpl->m_genomes[id];
}

void NEAT::Population_t::setFitness(int id, double fitness)
{
	if (id < 0 || id >= size())
		throw std::runtime_error(std::string("Invalid genome id passed to NEAT::Population_t::setFitness"));

	pimpl->m_genomes[id].setFitness(fitness);
}

std::vector<int> NEAT::Population_t::getSpeciesIds() const
{
	std::vector<int> rval;
	for (std::map<int, Impl::Species_t>::const_iterator it = pimpl->m_species_lists.begin(); it != pimpl->m_species_lists.end(); ++it)
		rval.push_back(it->first);

	return rval;
}

std::vector<int> NEAT::Population_t::getGenomeIdsOfSpecies(int speciesId) const
{
	std::map<int, Impl::Species_t>::const_iterator species_it = pimpl->m_species_lists.find(speciesId);

	std::vector<int> rval;
	if (species_it != pimpl->m_species_lists.end())
		std::copy(species_it->second.m_genomes.begin(), species_it->second.m_genomes.end(), std::back_inserter(rval));

	return rval;
}

std::vector<int> NEAT::Population_t::getBestGenomeIdsOfSpecies(int speciesId, double percentile) const
{
	std::vector<std::pair<int, double>> genome_id_to_fitness;
	std::map<int, Impl::Species_t>::const_iterator species_it = pimpl->m_species_lists.find(speciesId);
	if (species_it != pimpl->m_species_lists.end()) {
		for (std::set<int>::const_iterator genome_it = species_it->second.m_genomes.begin(); genome_it != species_it->second.m_genomes.end(); ++genome_it) {
			Genome_t& genome = pimpl->m_genomes[*genome_it];
			genome_id_to_fitness.push_back(std::make_pair(*genome_it, genome.getFitness()));
		}
	}

	std::sort(genome_id_to_fitness.begin(), genome_id_to_fitness.end(),
		[](const std::pair<int, double>& a, const std::pair<int, double>& b)
		{ return a.second > b.second; });
	int best_genome_num = (int)ceil(percentile * genome_id_to_fitness.size());
	std::vector<int> rval;
	for (int i = 0; i < best_genome_num; ++i)
		rval.push_back(genome_id_to_fitness[i].first);

	return rval;
}

int NEAT::Population_t::getFittestGenomeIdofSpecies(int speciesId) const
{
	int highestId = -1;
	double highestFitness = -1.0;
	
	std::vector<int> genome_list = getGenomeIdsOfSpecies(speciesId);
	for (int i = 0; i < genome_list.size(); ++i) {
		int testId = genome_list[i];
		double testFitness = getGenome(testId).getFitness();

		if (testFitness > highestFitness) {
			highestId = testId;
			highestFitness = testFitness;
		}
	}

	return highestId;
}

int NEAT::Population_t::getFittestGenomeId() const
{
	int highestId = -1;
	double highestFitness = -1.0;

	for (int i = 0; i < size(); ++i) {
		double testFitness = getGenome(i).getFitness();

		if (testFitness > highestFitness) {
			highestId = i;
			highestFitness = testFitness;
		}
	}

	return highestId;
}

int NEAT::Population_t::addToCorrectSpecies(const NEAT::Genome_t & genome, std::map<int, Genome_t>& representatives)
{
	for (std::map<int, Genome_t>::const_iterator rep_it = representatives.begin(); rep_it != representatives.end(); ++rep_it) {
		if (rep_it->second.delta(genome) <= pimpl->delta_t)
			return addToSpecificSpecies(genome, rep_it->first);
	}

	// this genome doesn't fit into any species defined last generation. Thus effective immediately, this is the representative of a new generation
	representatives[NEAT::Population_t::g_nextSpeciesNumber] = genome;
	return addToSpecificSpecies(genome, NEAT::Population_t::g_nextSpeciesNumber);
}

int NEAT::Population_t::addToSpecificSpecies(const NEAT::Genome_t & genome, int speciesId)
{
	int genomeId = (int)pimpl->m_genomes.size();
	pimpl->m_genomes.push_back(genome);

	// if the speciesId doesn't exist already, that's perfectly fine and an expected use case -- create the speciesId
	// HOWEVER, we must make sure to update the g_nextSpeciesNumber
	if (speciesId >= NEAT::Population_t::g_nextSpeciesNumber)
		NEAT::Population_t::g_nextSpeciesNumber = speciesId + 1;

	pimpl->m_species_lists[speciesId].m_genomes.insert(genomeId);
	return genomeId;
}

NEAT::Population_t NEAT::Population_t::createNextGeneration()
{
	NEAT::Population_t nextPop;
	
	nextPop.pimpl->m_generation_num = getGeneration() + 1;
	
	std::vector<int> current_species_list = getSpeciesIds();
	int speciesCnt = (int)current_species_list.size();

	// set species delta. This is adjusted up or down to guide the next population toward a desired number of species
	nextPop.pimpl->delta_t = pimpl->delta_t;
	if (nextPop.getGeneration() > 1) {
		if (speciesCnt < NEAT::Population_t::target_species_count)
			nextPop.pimpl->delta_t -= NEAT::Population_t::delta_t_step;
		else if (speciesCnt > NEAT::Population_t::target_species_count)
			nextPop.pimpl->delta_t += NEAT::Population_t::delta_t_step;

		// make sure we didn't accidentally force the delta too close to 0
		if (nextPop.pimpl->delta_t < NEAT::Population_t::delta_t_step)
			nextPop.pimpl->delta_t = NEAT::Population_t::delta_t_step;
	}
	
	std::vector<double> adjusted_fitness_list(speciesCnt, 0.0);
	std::map<int, NEAT::Genome_t> random_representatives;

	// smoosh together as much as we can into a single passthru
	for (int s = 0; s < speciesCnt; ++s) {
		int speciesId = current_species_list[s];
		std::vector<int> current_genome_list = getGenomeIdsOfSpecies(speciesId);
		int current_genome_list_size = (int)current_genome_list.size();

		// is this a stagnant species?
		double highestSpeciesFitness = getGenome(getFittestGenomeIdofSpecies(speciesId)).getFitness();
		pimpl->registerHighestFitness(speciesId, highestSpeciesFitness);

		for (int g = 0; g < current_genome_list_size; ++g) {
			// start calculation adjusted fitness... We'll use this later
			adjusted_fitness_list[s] += pimpl->calculateAdjustedFitness(current_genome_list[g], speciesId);
		}

		// randomly sample a single genome from each species
		int randomId = current_genome_list[randInt(0, current_genome_list_size - 1)];
		random_representatives[speciesId] = getGenome(randomId);
	}

	double total_adjusted_fitness = 0.0;
	for (int s = 0; s < speciesCnt; ++s) {
		total_adjusted_fitness += adjusted_fitness_list[s];
	}

	// figure out how many times each species is allowed to breed
	int nextPopSize = 0;
	std::vector<int> speciesAllotment(speciesCnt, 0);
	for (int s = 0; s < speciesCnt; ++s) {
		int allotment = (int)round(adjusted_fitness_list[s] / total_adjusted_fitness * NEAT::Population_t::genome_count);
		speciesAllotment[s] = allotment;
		nextPopSize += allotment;
	}

	// make sure the allotments didn't get screwed up due to rounding error
	if (nextPopSize != NEAT::Population_t::genome_count) {
		int to_modify = speciesCnt - 1;
		int to_modify_by = NEAT::Population_t::genome_count - nextPopSize;
		while ((to_modify > 0) && (speciesAllotment[to_modify] <= 1) && (speciesAllotment[to_modify] + to_modify_by <= 1)) {
			--to_modify;
		}
		speciesAllotment[to_modify] += to_modify_by;
	}

	for (int s = 0; s < speciesCnt; ++s) {
		int speciesId = current_species_list[s];
		std::vector<int> breedingPool = getBestGenomeIdsOfSpecies(speciesId, NEAT::Population_t::breeding_percentile);
		int breedingPoolCnt = (int)breedingPool.size();

		if (breedingPoolCnt > 0) {
			for (int i = 0; i < speciesAllotment[s]; ++i) {
				if (i == 0 && getGenomeIdsOfSpecies(speciesId).size() > NEAT::Population_t::min_species_size) {
					// carry across fittest member of each (sufficiently-sized) species
					int fittestGenomeId = getFittestGenomeIdofSpecies(speciesId);
					nextPop.addToSpecificSpecies(getGenome(fittestGenomeId), speciesId);
				}
				else {
					int id1 = breedingPool[randInt(0, breedingPoolCnt - 1)];
					int id2;

					if (randDouble() < interspecies_mating_rate) {
						// interspecies mating
						do {
							//TODO: the way I'm doing this is biased toward small species
							int randSpecies = randInt(0, speciesCnt - 1);
							std::vector<int> randBreedingPool = getBestGenomeIdsOfSpecies(current_species_list[randSpecies], NEAT::Population_t::breeding_percentile);
							id2 = randBreedingPool[randInt(0, (int)randBreedingPool.size() - 1)];
						} while (id1 == id2);
					}
					else {
						// normal mating
						if (breedingPoolCnt == 1) {
							// all by myseeeeeelf~
							id2 = id1;
						}
						else {
							do {
								id2 = breedingPool[randInt(0, breedingPoolCnt - 1)];
							} while (id1 == id2);
						}
					}

					// make babby, add it to the next generation
					Genome_t newOffspring = getGenome(id1).makeOffspring(getGenome(id2));
					nextPop.addToCorrectSpecies(newOffspring, random_representatives);
				}
			}
		}
	}

	return nextPop;
}

struct NEAT::Genome_t::Impl {
	Impl() : m_next_node_gene(0), m_max_innovation_mutation(0), m_fitness(0.0), m_adjusted_fitness(0.0) { }

	SparseMatrix_t<double> W_helper(int min_size);

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

SparseMatrix_t<double> NEAT::Genome_t::Impl::W_helper(int min_size)
{
	const int size = max(min_size, m_next_node_gene);
	SparseMatrix_t<double> rval(size, size);

	for (std::map<int, ConnectionGene_t>::const_iterator it = m_connection_genes.begin(); it != m_connection_genes.end(); ++it) {
		if (it->second.isEnabled()) {
			// yes, out is the row, in is the column. Since the state is a vertical vector, imagine input[0] = 1, and we have a connection (0, 4).
			// In that example we would expect node 4 to be lit in the resultant vector
			rval.addElem(it->second.getWeight(), it->second.getOutIndex(), it->second.getInIndex());
		}
	}

	return rval;
}

void NEAT::Genome_t::Impl::mutateAllConnectionWeights()
{
	for (std::map<int, ConnectionGene_t>::iterator it = m_connection_genes.begin(); it != m_connection_genes.end(); ++it) {
		if (randDouble() < Genome_t::mutate_all_connection_weights) {
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
		bool valid = true;
		do {
			inNum = node_keys[randInt(0, num_node_keys - 1)];
			outNum = node_keys[randInt(0, num_node_keys - 1)];

			std::map<std::pair<int, int>, ConnectionGene_t>::iterator all_it = g_all_connections.find(std::make_pair(inNum, outNum));
			if (all_it != g_all_connections.end()) {
				int innovationNum = all_it->second.getInnovationNumber();
				std::map<int, ConnectionGene_t>::iterator this_it = m_connection_genes.find(innovationNum);
				valid = (this_it == m_connection_genes.end());
			}
			else
				valid = true;
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
NEAT::Genome_t::Genome_t(Genome_t &&) noexcept = default;
NEAT::Genome_t & NEAT::Genome_t::operator=(Genome_t &&) noexcept = default;
NEAT::Genome_t::Genome_t(const NEAT::Genome_t & g) : pimpl(new Impl(*g.pimpl)) { }
NEAT::Genome_t & NEAT::Genome_t::operator=(const NEAT::Genome_t & g)
{
	if (this != &g) {
		pimpl.reset(new Impl(*g.pimpl));
	}

	return *this;
}
NEAT::Genome_t::~Genome_t() = default;

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

// with the way that these neural networks are designed, "inputs" really consist of intputs, outputs, and hidden.
// we can safely treat Wxh == Whh
SparseMatrix_t<double> NEAT::Genome_t::Wxh(int min_size) const
{
	return pimpl->W_helper(min_size);
}

SparseMatrix_t<double> NEAT::Genome_t::Whh(int min_size) const
{
	return pimpl->W_helper(min_size);
}

double NEAT::Genome_t::getFitness() const
{
	return pimpl->m_fitness;
}

void NEAT::Genome_t::setFitness(double fitness)
{
	pimpl->m_fitness = fitness;
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

	// this need to be recalculated, zero them out
	rval.setFitness(0.0);

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
	pimpl->mutateAllConnectionWeights();

	// should we add a brand new connection?
	if (randDouble() < Genome_t::mutate_add_connection) {
		std::pair<int, int> newConnectionPair = pimpl->findNewAddConnection();
		if (newConnectionPair.first != -1)
			addConnectionGene(newConnectionPair.first, newConnectionPair.second, getNormalizedRand());
	}

	//TODO: Consider a mutation that connects one input to EVERY output node. It's a good mutation to jump-start from gen #0

	// should we add a brand new node?
	if (randDouble() < Genome_t::mutate_add_node) {
		// adding a new node requires taking an existing connection and splitting it. Thus, it makes no sense for (starting) networks with zero connections
		if (pimpl->m_connection_genes.size() > 0) {
			int newNodeIndex = addNodeGene(NEAT::HIDDEN_NODE, "Hidden");
			if (newNodeIndex != -1) { // no errors
				NEAT::ConnectionGene_t oldConnectionCopy = pimpl->findConnectionAddNewNode();
				addConnectionGene(oldConnectionCopy.getInIndex(), newNodeIndex, 1.0);
				addConnectionGene(newNodeIndex, oldConnectionCopy.getOutIndex(), oldConnectionCopy.getWeight());
			}
		}
	}
}

struct NEAT::NodeGene_t::Impl {
	Impl(int index, NODETYPE nodetype, const std::string& label);

	int m_index;
	NODETYPE m_nodetype;
	std::string m_label;
};

NEAT::NodeGene_t::Impl::Impl(int index, NODETYPE nodetype, const std::string& label)
	: m_index(index), m_nodetype(nodetype), m_label(label)
{
}

NEAT::NodeGene_t::NodeGene_t(int index, NODETYPE nodetype, const std::string & label) : pimpl(new Impl(index, nodetype, label)) { }
NEAT::NodeGene_t::NodeGene_t(NEAT::NodeGene_t &&) noexcept = default;
NEAT::NodeGene_t & NEAT::NodeGene_t::operator=(NEAT::NodeGene_t &&) noexcept = default;
NEAT::NodeGene_t::NodeGene_t(const NEAT::NodeGene_t & ng) : pimpl(new Impl(*ng.pimpl)) { }
NEAT::NodeGene_t & NEAT::NodeGene_t::operator=(const NEAT::NodeGene_t & ng)
{
	if (this != &ng)
		pimpl.reset(new Impl(*ng.pimpl));

	return *this;
}
NEAT::NodeGene_t::~NodeGene_t() = default;

std::string NEAT::NodeGene_t::getLabel() const
{
	return pimpl->m_label;
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

void NEAT::ConnectionGene_t::setWeight(double weight)
{
	m_weight = weight;
}

bool NEAT::ConnectionGene_t::isEquivalentConnection(const ConnectionGene_t & c) const
{
	return isEquivalentConnection(c.getInIndex(), c.getOutIndex());
}

bool NEAT::ConnectionGene_t::isEquivalentConnection(int in, int out) const
{
	return (m_in_index == in) && (m_out_index == out);
}
