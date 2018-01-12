// main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "rdm_moves.h"
#include "rdm_state.h"



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

int main()
{
/*    State_t state;
    state.m_statics.m_gcd = 2420;
    state.m_statics.m_test_duration = 15 * 60 * 1000;

    std::shared_ptr<Move_t> jolt2(new Jolt2_t());

    MoveStates_t movestates(state);

    while (!movestates.isFinished())
        movestates.advanceAndAddMove(jolt2);

    movestates.finished();

    std::cout << std::distance(movestates.begin(), movestates.end()) << std::endl;
    movestates.print(std::cout, MoveStates_t::MS_PRINT_DPS, '\t');
    //movestates.print(std::cout, MoveStates_t::MS_PRINT_CHARLIST, ' ', 3);*/
    MoveStates_t movestates = getRaid5thGCDEmboldenOpener();
    //movestates.advanceAndAddMove(std::shared_ptr<Move_t>(new EndPlaceholder_t()));
    movestates.finish();
    movestates.print(std::cout, MoveStates_t::MS_PRINT_DPS, '\t');
    return 0;
}

