#include "stdafx.h"
#include "NEATGenome.h"

int NEAT::ConnectionGene_t::g_nextInnovationNumber = 0;

struct NEAT::Population_t::Impl {
	std::map<int, Species_t> m_species;
};

NEAT::Population_t::Population_t()
	: pimpl(new Impl())
{
}

NEAT::Population_t::~Population_t()
{
	delete pimpl;
}

void NEAT::Population_t::addToCorrectSpecies(const Genome_t & genome)
{
	throw;
}

struct NEAT::Species_t::Impl {
	std::map<int, NEAT::Genome_t> m_genomes;
};

NEAT::Species_t::Species_t()
	: pimpl(new Impl())
{
}

NEAT::Species_t::~Species_t()
{
	delete pimpl;
}

struct NEAT::Genome_t::Impl {
	// don't use shared_ptr, we want our own copies to modify values
	std::map<int, NodeGene_t> m_node_genes;
	std::map<int, ConnectionGene_t>  m_connection_genes;
};

NEAT::Genome_t::Genome_t()
	: pimpl(new Impl())
{
	// generate all standardized node genes
	// use enum values so that I can always figure out what node maps to what
}

NEAT::Genome_t::~Genome_t()
{
	delete pimpl;
}

void NEAT::Genome_t::addNodeGene(int num, NODETYPE nodetype, const std::string & label)
{
	throw;
}

void NEAT::Genome_t::addConnectionGene(int in, int out, double weight)
{
	throw;
}

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

NEAT::NodeGene_t::NodeGene_t(int index, NODETYPE nodetype, const std::string & label)
	: pimpl(new Impl(index, nodetype, label))
{
	throw;
}

NEAT::NodeGene_t::~NodeGene_t()
{
	delete pimpl;
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
	  m_innovation_number(c.m_innovation_number)
{
	// we transfer the innovation number. This is a true copy, not a clone
}

bool NEAT::ConnectionGene_t::isEquivalentConnection(const ConnectionGene_t & c)
{
	return isEquivalentConnection(c.getInIndex(), c.getOutIndex());
}

bool NEAT::ConnectionGene_t::isEquivalentConnection(int in, int out)
{
	return (m_in_index == in) && (m_out_index == out);
}
