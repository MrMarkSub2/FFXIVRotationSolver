#include "stdafx.h"
#include "CppUnitTest.h"
#include "rdm_moves.h"
#include <string>
#include <memory>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestCases
{
    TEST_CLASS(MoveTests)
    {
    public:
        TEST_METHOD(Basics)
        {
            std::shared_ptr<Move_t> move(new Jolt2_t());
            State_t state;

            Assert::IsTrue(move->IsUseable(state));
            Assert::AreEqual(240, move->CalculatePotency(state));

            Assert::AreEqual(0, state.m_status.m_impactful);
            move->Execute(state);
            Assert::AreNotEqual(0, state.m_status.m_impactful);
        }

        TEST_METHOD(SpellChainsImpact) {
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> imp(new Impact_t());
            State_t state;

            Assert::IsFalse(imp->IsUseable(state));
            jolt2->Execute(state);
            Assert::IsTrue(imp->IsUseable(state));
            imp->Execute(state);
            Assert::IsFalse(imp->IsUseable(state));
        }

        TEST_METHOD(SpellChainsVerfire) {
            State_t state;
            std::shared_ptr<Move_t> verfire(new Verfire_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());

            Assert::IsFalse(verfire->IsUseable(state));

            // astronomically low odds that we cast 100x Verthunder without a proc
            for (int i = 0; i < 100; ++i) {
                verthunder->Execute(state);
                if (verfire->IsUseable(state)) {
                    verfire->Execute(state);
                    Assert::IsFalse(verfire->IsUseable(state));
                    return;
                }
            }

            Assert::Fail();
        }

        void breaksMeleeHelper(std::shared_ptr<Move_t> move, bool shouldBreak) {
            State_t state;
            state.m_caststate.m_out_of_range = 0;
            state.m_mana.add(80, 80);

            std::shared_ptr<Move_t> rip(new EnhRiposte_t());
            std::shared_ptr<Move_t> zwer(new EnhZwerchhau_t());
            std::shared_ptr<Move_t> redoub(new EnhRedoublement_t());
            std::shared_ptr<Move_t> verholy(new Verholy_t());

            Assert::IsTrue(rip->IsUseable(state), L"Riposte not usable!");
            rip->Execute(state);
            Assert::IsTrue(zwer->IsUseable(state), L"Zwer not usable!");
            zwer->Execute(state);
            move->Execute(state);
            redoub->Execute(state);
            if (shouldBreak)
                Assert::IsFalse(verholy->IsUseable(state), convert_to_wc((std::string("Expected ") + move->getName() + std::string(" to break!")).c_str()));
            else
                Assert::IsTrue(verholy->IsUseable(state), convert_to_wc((std::string("Expected ") + move->getName() + std::string(" to not break!")).c_str()));
        }

        TEST_METHOD(BreaksMelee) {
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Jolt2_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Impact_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Verthunder_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Veraero_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Verfire_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Verstone_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Scatter_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Manification_t()), true);

            breaksMeleeHelper(std::shared_ptr<Move_t>(new Corps_t()), false);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Displacement_t()), false);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Acceleration_t()), false);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Embolden_t()), false);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Swiftcast_t()), false);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Fleche_t()), false);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Contre_t()), false);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Infusion_t()), false);
        }
    };

    MoveString_t createMovestringMeleeFinisher() {
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
    }

    TEST_CLASS(MoveStringTests)
    {
    public:
        TEST_METHOD(Empty)
        {
            MoveString_t movestring;
            Assert::IsTrue(movestring.begin() == movestring.end());
        }

        TEST_METHOD(IterateAbilityChars)
        {
            MoveString_t movestring = createMovestringMeleeFinisher();
            MoveString_t::container_t::const_iterator it = movestring.begin();

            Assert::AreEqual(std::string("Ri"), (*it++)->getChar());
            Assert::AreEqual(std::string("^"), (*it++)->getChar());
            Assert::AreEqual(std::string("Z"), (*it++)->getChar());
            Assert::AreEqual(std::string("L"), (*it++)->getChar());
            Assert::AreEqual(std::string("Re"), (*it++)->getChar());
            Assert::AreEqual(std::string("v"), (*it++)->getChar());
            Assert::AreEqual(std::string("B"), (*it++)->getChar());
            Assert::AreEqual(std::string("J"), (*it++)->getChar());
            Assert::AreEqual(std::string("END"), (*it++)->getChar());
        }
    };

    TEST_CLASS(MoveStatesTests)
    {
    public:
        TEST_METHOD(Empty)
        {
            State_t state;
            MoveStates_t movestates(state);
            Assert::IsTrue(movestates.begin() == movestates.end());
        }

        TEST_METHOD(SimpleOpener) {
            State_t opener_state;
            opener_state.m_statics.m_gcd = 2420;
            MoveStates_t opener_ms(opener_state);

            std::shared_ptr<Move_t> accel(new Acceleration_t());
            std::shared_ptr<Move_t> div(new DiversionPlaceholder_t());
            std::shared_ptr<Move_t> veraero(new Veraero_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> end(new EndPlaceholder_t());

            opener_ms.advanceAndAddMove(accel);
            opener_ms.advance(4000);
            opener_ms.advanceAndAddMove(div);
            opener_ms.advance((-5000 * opener_state.m_statics.m_gcd / 25000 * 10) - (-6000)); // butt it right up against start time
            opener_ms.advanceAndAddMove(veraero);

            Assert::AreEqual(10000, opener_ms.constLastState().m_total_duration + veraero->getCast() * opener_state.m_statics.m_gcd / 25000 * 10);

            MoveStates_t movestates(opener_ms, -10 * 1000);
            movestates.advance(movestates.constLastState().m_caststate.m_casting);

            Assert::AreEqual(0, movestates.constLastState().m_total_duration);
            Assert::IsTrue(movestates.constLastState().m_status.m_dualcast > 0, L"dualcast not active");
            Assert::IsTrue(movestates.constLastState().m_status.m_verstone > 0, L"guaranteed verstone not active");

            movestates.advanceAndAddMove(verthunder);
            movestates.advanceAndAddMove(end);

            Assert::AreEqual(opener_state.m_statics.m_gcd, movestates.constLastState().m_total_duration);
            Assert::AreEqual(600, movestates.constLastState().m_total_potency);
        }

        TEST_METHOD(Raid5thGCDEmboldenOpener) {
            State_t opener_state;
            opener_state.m_statics.m_gcd = 2420;
            opener_state.m_caststate.m_clipping = -1400; // offset allegedly clipping in precast accel/div
            MoveStates_t opener_ms(opener_state);

            std::shared_ptr<Move_t> accel(new Acceleration_t());
            std::shared_ptr<Move_t> div(new DiversionPlaceholder_t());
            std::shared_ptr<Move_t> veraero(new Veraero_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> infusion(new Infusion_t());
            std::shared_ptr<Move_t> verstone(new Verstone_t());
            std::shared_ptr<Move_t> verfire(new Verfire_t());
            std::shared_ptr<Move_t> fleche(new Fleche_t());
            std::shared_ptr<Move_t> contre(new Contre_t());
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> impact(new Impact_t());
            std::shared_ptr<Move_t> swift(new Swiftcast_t());
            std::shared_ptr<Move_t> embolden(new Embolden_t());
            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> disp(new Displacement_t());
            std::shared_ptr<Move_t> mani(new Manification_t());
            std::shared_ptr<Move_t> rip(new EnhRiposte_t());
            std::shared_ptr<Move_t> zwer(new EnhZwerchhau_t());
            std::shared_ptr<Move_t> redoub(new EnhRedoublement_t());
            std::shared_ptr<Move_t> verflare(new Verflare_t());
            std::shared_ptr<Move_t> end(new EndPlaceholder_t());

            //const State_t& laststateref = opener_ms.constLastState();

            opener_ms.advanceAndAddMove(accel);
            opener_ms.advance(4000);
            opener_ms.advanceAndAddMove(div);
            opener_ms.advance((-5000 * opener_state.m_statics.m_gcd / 25000 * 10) - (-6000)); // butt it right up against start time
            opener_ms.advanceAndAddMove(veraero); // 300 | 0

            Assert::AreEqual(10000, opener_ms.constLastState().m_total_duration + veraero->getCast() * opener_state.m_statics.m_gcd / 25000 * 10);

            opener_ms.advanceAndAddMove(verthunder); // 300 | 2.42
            opener_ms.advanceAndAddMove(infusion); //  | 2.42 no clip
            opener_ms.advanceAndAddMove(verstone); // 284 | 4.84
            opener_ms.advanceAndAddMove(veraero); // 316 | 7.26
            opener_ms.advanceAndAddMove(fleche); // 442 | 7.26 no clip
            opener_ms.advanceAndAddMove(contre); // 316 | 7.26 no clip
            opener_ms.advanceAndAddMove(jolt2); // 253 | 9.68 
            opener_ms.advanceAndAddMove(verthunder); // 316 | 12.10
            opener_ms.advanceAndAddMove(swift); //  | 12.10 no clip
            opener_ms.advanceAndAddMove(embolden); //  | 12.10 no clip | EMBOLDEN 5
            opener_ms.advanceAndAddMove(verthunder); // 347 | 14.52

            Assert::AreEqual(0, opener_ms.constLastState().m_caststate.m_clipping, L"already clipping!");

            opener_ms.advanceAndAddMove(disp); // 137 | 14.52 no clip
            opener_ms.advanceAndAddMove(corps); // 137 | 14.85 clipping!
            opener_ms.advance(opener_ms.constLastState().m_caststate.m_animation_lock);

            Assert::AreEqual(330, opener_ms.constLastState().m_caststate.m_clipping, L"didn't clip!");

            opener_ms.advanceAndAddMove(impact); // 307 | 17.27 | EMBOLDEN 4
            opener_ms.advanceAndAddMove(veraero); // 341 | 19.69
            opener_ms.advanceAndAddMove(div); // actually Manashift but w/e
            opener_ms.advanceAndAddMove(mani); //  | 19.69 no clip
            opener_ms.advanceAndAddMove(rip); // 234 | 21.19 | EMBOLDEN 3
            opener_ms.advanceAndAddMove(corps); // 137 | 21.19 no clip
            opener_ms.advanceAndAddMove(zwer); // 324 | 22.69
            opener_ms.advanceAndAddMove(redoub); // 525 | 24.89
            opener_ms.advanceAndAddMove(disp); // 137 | 24.89 no clip
            opener_ms.advanceAndAddMove(verflare); // 603 | 27.31 | EMBOLDEN 2
            opener_ms.advanceAndAddMove(accel); //  | 27.31 no clip
            opener_ms.advanceAndAddMove(verfire); // 290 | 29.73 | EMBOLDEN 1
            opener_ms.advanceAndAddMove(veraero); // 322 | 32.15
            opener_ms.advanceAndAddMove(fleche); // 442 | 32.15 no clip
            Assert::AreEqual(42150, opener_ms.constLastState().m_total_duration + opener_ms.constLastState().m_caststate.m_gcd, L"running duration is off");
            // infusion wears off AFTER fleche
            // embolden wears off AFTER embolden

            MoveStates_t movestates(opener_ms, -10 * 1000);

            movestates.advanceAndAddMove(end);

            Assert::AreEqual(32150, movestates.constLastState().m_total_duration, L"total duration is off");
            Assert::AreEqual(6810, movestates.constLastState().m_total_potency);
            AssertIsEssentuallyEqual(6810.0 / 32150.0 * 1000.0, movestates.constLastState().getDPS().calc());
        }

        MoveStates_t createMovestatesMeleeFinisher() {
            State_t state;
            state.m_mana.add(80, 80);
            state.m_caststate.m_out_of_range = 0;
            state.m_statics.m_gcd = 2420;

            MoveStates_t movestates(state);

            std::shared_ptr<Move_t> rip(new EnhRiposte_t());
            std::shared_ptr<Move_t> zwer(new EnhZwerchhau_t());
            std::shared_ptr<Move_t> redoub(new EnhRedoublement_t());
            std::shared_ptr<Move_t> verflare(new Verflare_t());

            std::shared_ptr<Move_t> fleche(new Fleche_t());
            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> disp(new Displacement_t());
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());

            std::shared_ptr<Move_t> end(new EndPlaceholder_t());

            movestates.advanceAndAddMove(rip);
            movestates.advanceAndAddMove(corps);

            movestates.advanceAndAddMove(zwer);
            movestates.advanceAndAddMove(fleche);

            movestates.advanceAndAddMove(redoub);
            movestates.advanceAndAddMove(disp);

            movestates.advanceAndAddMove(verflare);

            movestates.advanceAndAddMove(jolt2);

            movestates.advanceAndAddMove(end);

            return movestates;
        }

        TEST_METHOD(IterateAbilityChars)
        {
            MoveStates_t movestates = createMovestatesMeleeFinisher();
            MoveStates_t::container_t::const_iterator it = movestates.begin();

            Assert::AreEqual(std::string("Ri"), it++->first->getChar());
            Assert::AreEqual(std::string("^"), it++->first->getChar());
            Assert::AreEqual(std::string("Z"), it++->first->getChar());
            Assert::AreEqual(std::string("L"), it++->first->getChar());
            Assert::AreEqual(std::string("Re"), it++->first->getChar());
            Assert::AreEqual(std::string("v"), it++->first->getChar());
            Assert::AreEqual(std::string("B"), it++->first->getChar());
            Assert::AreEqual(std::string("J"), it++->first->getChar());
            Assert::AreEqual(std::string("END"), it++->first->getChar());
        }

        TEST_METHOD(IteratePotency)
        {
            MoveStates_t movestates = createMovestatesMeleeFinisher();
            MoveStates_t::container_t::const_iterator it = movestates.begin();

            int pot = 210; // Ri
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 130; // ^
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 290; // Z
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 420; // L
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 470; // Re
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 130; // v
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 550; // B
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 0; // J is still casting, no potency increase yet!
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 240; // END
            Assert::AreEqual(pot, it++->second.m_total_potency);
        }

        TEST_METHOD(IterateDuration)
        {
            MoveStates_t movestates = createMovestatesMeleeFinisher();
            MoveStates_t::container_t::const_iterator it = movestates.begin();

            int duration = 0; // Ri
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 700; // ^ in Ri
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 800; // Z, Ri done
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 700; // L in Z
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 800; // Re, Z done
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 700; // v in Re
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 1500; // B, Re done
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 2420; // J, B done
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 2420; // END
            Assert::AreEqual(duration, it++->second.m_total_duration);
        }

        TEST_METHOD(IterateAbilityChars_MoveStringLoad)
        {
            State_t state;
            state.m_mana.add(80, 80);
            state.m_caststate.m_out_of_range = 0;
            state.m_statics.m_gcd = 2420;

            MoveStates_t movestates(state);
            movestates.advanceAndAddMoveString(createMovestringMeleeFinisher());
            MoveStates_t::container_t::const_iterator it = movestates.begin();

            Assert::AreEqual(std::string("Ri"), it++->first->getChar());
            Assert::AreEqual(std::string("^"), it++->first->getChar());
            Assert::AreEqual(std::string("Z"), it++->first->getChar());
            Assert::AreEqual(std::string("L"), it++->first->getChar());
            Assert::AreEqual(std::string("Re"), it++->first->getChar());
            Assert::AreEqual(std::string("v"), it++->first->getChar());
            Assert::AreEqual(std::string("B"), it++->first->getChar());
            Assert::AreEqual(std::string("J"), it++->first->getChar());
            Assert::AreEqual(std::string("END"), it++->first->getChar());
        }

        TEST_METHOD(IteratePotency_MoveStringLoad)
        {
            State_t state;
            state.m_mana.add(80, 80);
            state.m_caststate.m_out_of_range = 0;
            state.m_statics.m_gcd = 2420;

            MoveStates_t movestates(state);
            movestates.advanceAndAddMoveString(createMovestringMeleeFinisher());
            MoveStates_t::container_t::const_iterator it = movestates.begin();

            int pot = 210; // Ri
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 130; // ^
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 290; // Z
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 420; // L
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 470; // Re
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 130; // v
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 550; // B
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 0; // J is still casting, no potency increase yet!
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 240; // END
            Assert::AreEqual(pot, it++->second.m_total_potency);
        }

        TEST_METHOD(IterateDuration_MoveStringLoad)
        {
            State_t state;
            state.m_mana.add(80, 80);
            state.m_caststate.m_out_of_range = 0;
            state.m_statics.m_gcd = 2420;

            MoveStates_t movestates(state);
            movestates.advanceAndAddMoveString(createMovestringMeleeFinisher());
            MoveStates_t::container_t::const_iterator it = movestates.begin();

            int duration = 0; // Ri
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 700; // ^ in Ri
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 800; // Z, Ri done
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 700; // L in Z
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 800; // Re, Z done
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 700; // v in Re
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 1500; // B, Re done
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 2420; // J, B done
            Assert::AreEqual(duration, it++->second.m_total_duration);
            duration += 2420; // END
            Assert::AreEqual(duration, it++->second.m_total_duration);
        }

        TEST_METHOD(FinishIt) {
            State_t state;
            state.m_statics.m_gcd = 2420;
            state.m_statics.m_test_duration = 5 * 60 * 1000;

            std::shared_ptr<Move_t> jolt2(new Jolt2_t());

            MoveStates_t movestates(state);

            while (!movestates.isFinished())
                movestates.advanceAndAddMove(jolt2);

            movestates.finish();

            Assert::AreEqual(125, (int)std::distance(movestates.begin(), movestates.end()));
            Assert::IsTrue(movestates.isFinished());
        }

        TEST_METHOD(FinishedByMove) {
            State_t state;
            state.m_statics.m_gcd = 2420;
            state.m_statics.m_test_duration = 5 * 60 * 1000;

            std::shared_ptr<Move_t> end(new EndPlaceholder_t());

            MoveStates_t movestates(state);
            movestates.advanceAndAddMove(end);

            Assert::IsTrue(movestates.isFinished());
        }
    };
}