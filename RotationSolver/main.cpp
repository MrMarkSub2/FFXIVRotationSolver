// main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "xor.h"
#include <iostream>
#include <iomanip>
//#include <string>
#include "NEATGenome.h"
//#include "../NEAT/utils.h"

/*MoveString_t createMovestringMeleeFinisher() {
    MoveString_t movestring;

    std::shared_ptr<Move_t> rip(new EnhRiposte_t());
    std::shared_ptr<Move_t> zwer(new EnhZwerchhau_t());
    std::shared_ptr<Move_t> redoub(new EnhRedoublement_t());
    std::shared_ptr<Move_t> verflare(new Verflare_t());

    std::shared_ptr<Move_t> fleche(new Fleche_t());
    std::shared_ptr<Move_t> corps(new Corps_t());
    std::shared_ptr<Move_t> disp(new Displacement_t());
    std::shared_ptr<Move_t> jolt2(new Jolt2_t());

    std::shared_ptr<Move_t> end(new EndPlaceholder_t());

    movestring.addMove(rip);
    movestring.addMove(corps);
    movestring.addMove(zwer);
    movestring.addMove(fleche);
    movestring.addMove(redoub);
    movestring.addMove(disp);
    movestring.addMove(verflare);
    movestring.addMove(jolt2);
    movestring.addMove(end);

    return movestring;
}*/

//void testNESTNextGen() {
//
//	NEAT::Population_t pop;
//	NEAT::Genome_t g1;
//	int in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #1");
//	int out_num = g1.addNodeGene(NEAT::OUTPUT_NODE, "Out #1");
//	int connection1 = g1.addConnectionGene(in_num, out_num, NEAT::getNormalizedRand());
//	in_num = g1.addNodeGene(NEAT::INPUT_NODE, "In #2");
//	int connection2 = g1.addConnectionGene(in_num, out_num, NEAT::getNormalizedRand());
//
//	for (int i = 0; i < 20; ++i) {
//		g1.updateConnectionGene(connection1, NEAT::getNormalizedRand());
//		g1.updateConnectionGene(connection2, NEAT::getNormalizedRand());
//		int id = pop.addToSpecificSpecies(g1, 0);
//		pop.setFitness(id, id);
//	}
//
//	for (int i = 0; i < 500; ++i) {
//		pop = pop.createNextGeneration();
//		std::cout << pop.getGeneration() << std::endl;
//	}
//}

int main()
{
	//for (int i = 0; i < 1000; ++i)
	//	std::cout << NEAT::getNormalizedRand(0.0, 1.0) << std::endl;
	xorSolver_t xSolve;
	for (int i = 0; i < 1000; ++i) {
		xSolve.evaluateGeneration();
		NEAT::Genome_t bestG = xSolve.getBestOfGeneration();
		std::cout << "Generation: " << xSolve.getGeneration() << "   Fitness: " << bestG.getFitness() << "   Nodes: " << bestG.Wxh().rowCount() << "   Connect: " << bestG.Wxh().size() << std::endl;
		//TODO: Function to evaluate bestG to see if it passes all 4, break
		//for (int j = 0; j < bestG.Wxh().m_elems.size(); ++j)
		//	std::cout << "\t(" << bestG.Wxh().m_elems[j].col << "," << bestG.Wxh().m_elems[j].row << ") = " << bestG.Wxh().m_elems[j].val << std::endl;
		
		int successes = 0;
		for (int t = 0; t < 4; ++t) {
			std::vector<double> input = { (double)((t & 0x2) >> 1), (double)(t & 0x1) };
			double answer = ((t & 0x2) >> 1) ^ (t & 0x1);
			std::vector<double> output = xSolve.evaluate(bestG, input);
			bool match = (abs(output[0] - answer) < 0.5);
			if (match)
				++successes;

			std::cout << "\t" << ((t & 0x2) >> 1) << " ^ " << (t & 0x1) << " = " << std::setprecision(3) << std::fixed << output[0] << "\t(" << std::boolalpha << match << ")" << std::endl;
		}

		if (successes == 4) {
			std::cout << "\tSUCCESS!" << std::endl;
			break;
		}
		
		xSolve.nextGeneration();
	}

	return 0;

	//MoveString_t movestring;
	//std::shared_ptr<Move_t> jolt2(new Jolt2_t());
	//std::shared_ptr<Move_t> verthunder(new Verthunder_t());
	//std::shared_ptr<Move_t> veraero(new Veraero_t());
	//std::shared_ptr<Move_t> rip(new EnhRiposte_t());
	//std::shared_ptr<Move_t> zwer(new EnhZwerchhau_t());
	//std::shared_ptr<Move_t> redoub(new EnhRedoublement_t());
	//std::shared_ptr<Move_t> verflare(new Verflare_t());

	//movestring.addMove(jolt2);
	//movestring.addMove(jolt2);
	////movestring.addMove(jolt2);
	////movestring.addMove(veraero);
	////movestring.addMove(jolt2);
	////movestring.addMove(verthunder);
	////movestring.addMove(jolt2);
	////movestring.addMove(veraero);
	////movestring.addMove(jolt2);
	////movestring.addMove(verthunder);
	////movestring.addMove(jolt2);
	////movestring.addMove(veraero);
	////movestring.addMove(jolt2);
	////movestring.addMove(verthunder);
	////movestring.addMove(jolt2);
	////movestring.addMove(veraero);
	////movestring.addMove(jolt2);
	////movestring.addMove(verthunder);
	////movestring.addMove(jolt2);
	////movestring.addMove(veraero);
	////movestring.addMove(jolt2);
	////movestring.addMove(verthunder);
	////movestring.addMove(rip);
	////movestring.addMove(zwer);
	////movestring.addMove(redoub);
	////movestring.addMove(verflare);

	//State_t state;
	//MoveStates_t movestates(state);
	//movestates.advanceAndAddMoveString(movestring);
	//movestates.finish();
	//movestates.print(std::cout, MoveStates_t::MS_PRINT_DPS, '\t');
    return 0;
}

