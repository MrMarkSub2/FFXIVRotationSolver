#include "stdafx.h"
#include "CppUnitTest.h"
#include "rdm_moves.h"
#include <string>
#include <memory>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RDMModelTestCases
{
    TEST_CLASS(MoveTests)
    {
    public:
        TEST_METHOD(Basics)
        {
            std::shared_ptr<Move_t> move(new Jolt2_t());
            State_t state;

            Assert::IsTrue(move->IsUseable(state));
            Assert::AreEqual(280, move->CalculatePotency(state));
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
			breaksMeleeHelper(std::shared_ptr<Move_t>(new Verthunder2_t()), true);
			breaksMeleeHelper(std::shared_ptr<Move_t>(new Veraero2_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Verfire_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Verstone_t()), true);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Manification_t()), true);
			breaksMeleeHelper(std::shared_ptr<Move_t>(new EnhReprise_t()), true);


            breaksMeleeHelper(std::shared_ptr<Move_t>(new Corps_t()), false);
            breaksMeleeHelper(std::shared_ptr<Move_t>(new Displacement_t()), false);
			breaksMeleeHelper(std::shared_ptr<Move_t>(new Engagement_t()), false);
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
		std::shared_ptr<Move_t> scorch(new Scorch_t());

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
		movestring.addMove(scorch);
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
			Assert::AreEqual(std::string("S"), (*it++)->getChar());
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
            std::shared_ptr<Move_t> veraero(new Veraero_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> end(new EndPlaceholder_t());

            opener_ms.advanceAndAddMove(accel);
            opener_ms.advance((-5000 * opener_state.m_statics.m_gcd / 25000 * 10) - (-14900)); // butt it right up against start time
            opener_ms.advanceAndAddMove(veraero);

            Assert::AreEqual(14900, opener_ms.constLastState().m_total_duration + veraero->getCast() * opener_state.m_statics.m_gcd / 25000 * 10);

            MoveStates_t movestates(opener_ms, -14900);
            movestates.advance(movestates.constLastState().m_caststate.m_casting);

            Assert::AreEqual(0, movestates.constLastState().m_total_duration);
            Assert::IsTrue(movestates.constLastState().m_status.m_dualcast > 0, L"dualcast not active");
            Assert::IsTrue(movestates.constLastState().m_status.m_verstone > 0, L"guaranteed verstone not active");

            movestates.advanceAndAddMove(verthunder);
            movestates.advanceAndAddMove(end);

            Assert::AreEqual(opener_state.m_statics.m_gcd, movestates.constLastState().m_total_duration);
            Assert::AreEqual(740, movestates.constLastState().m_total_potency);
        }

        TEST_METHOD(Raid3rdGCDEmboldenOpener) {
            State_t opener_state;
            opener_state.m_statics.m_gcd = 2420;
            opener_state.m_caststate.m_clipping = -700; // offset allegedly clipping in precast accel/div
            MoveStates_t opener_ms(opener_state);

            std::shared_ptr<Move_t> accel(new Acceleration_t());
            std::shared_ptr<Move_t> veraero(new Veraero_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> infusion(new Infusion_t());
            std::shared_ptr<Move_t> verstone(new Verstone_t());
            std::shared_ptr<Move_t> verfire(new Verfire_t());
            std::shared_ptr<Move_t> fleche(new Fleche_t());
            std::shared_ptr<Move_t> contre(new Contre_t());
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> swift(new Swiftcast_t());
            std::shared_ptr<Move_t> embolden(new Embolden_t());
            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> disp(new Displacement_t());
			std::shared_ptr<Move_t> engage(new Engagement_t());
            std::shared_ptr<Move_t> mani(new Manification_t());
            std::shared_ptr<Move_t> rip(new EnhRiposte_t());
            std::shared_ptr<Move_t> zwer(new EnhZwerchhau_t());
            std::shared_ptr<Move_t> redoub(new EnhRedoublement_t());
            std::shared_ptr<Move_t> verflare(new Verflare_t());
			std::shared_ptr<Move_t> scorch(new Scorch_t());
            std::shared_ptr<Move_t> end(new EndPlaceholder_t());

            //const State_t& laststateref = opener_ms.constLastState();

            opener_ms.advanceAndAddMove(accel);
            opener_ms.advance((-5000 * opener_state.m_statics.m_gcd / 25000 * 10) - (-14900)); // butt it right up against start time
			//TODO: Need to fix ALL of these running potency/gcd comments
            opener_ms.advanceAndAddMove(veraero); // 370 | 0

            Assert::AreEqual(14900, opener_ms.constLastState().m_total_duration + veraero->getCast() * opener_state.m_statics.m_gcd / 25000 * 10);

            opener_ms.advanceAndAddMove(verthunder); // 370 | 2.42
            opener_ms.advanceAndAddMove(infusion); //  | 2.42 no clip
            opener_ms.advanceAndAddMove(verstone); // 316 | 4.84
            opener_ms.advanceAndAddMove(veraero); // 390 | 7.26
            opener_ms.advanceAndAddMove(fleche); // 463 | 7.26 no clip
			opener_ms.advanceAndAddMove(embolden); //  | 7.26 no clip | EMBOLDEN 5
			opener_ms.advanceAndAddMove(verfire); // 347 | 9.68
            opener_ms.advanceAndAddMove(verthunder); // 429 | 12.10
			opener_ms.advanceAndAddMove(corps); // 137 | 12.10 no clip
			opener_ms.advanceAndAddMove(contre); // 421 | 12.10 no clip
			opener_ms.advanceAndAddMove(verstone); // 341 | 14.52 | EMBOLDEN 3
            opener_ms.advanceAndAddMove(verthunder); // 413 | 16.94

            Assert::AreEqual(0, opener_ms.constLastState().m_caststate.m_clipping, L"already clipping!");

            opener_ms.advanceAndAddMove(engage); // 158 | 16.94 no clip
            opener_ms.advanceAndAddMove(mani); //  | 16.94 no clip
            opener_ms.advanceAndAddMove(rip); // 234 | 18.44
            opener_ms.advanceAndAddMove(corps); // 137 | 18.44 no clip
            opener_ms.advanceAndAddMove(zwer); // 318 | 19.94
            opener_ms.advanceAndAddMove(redoub); // 515 | 22.14 | EMBOLDEN 2
            opener_ms.advanceAndAddMove(disp); // 210 | 22.14 no clip
            opener_ms.advanceAndAddMove(verflare); // 658 | 24.56
            opener_ms.advanceAndAddMove(scorch); // 752 | 26.98 | EMBOLDEN 1
            opener_ms.advanceAndAddMove(verfire); // 316 | 29.40 | EMBOLDEN 0
            opener_ms.advanceAndAddMove(veraero); // 390 | 31.82
            opener_ms.advanceAndAddMove(fleche); // 463 | 31.82 no clip

			Assert::AreEqual(0, opener_ms.constLastState().m_caststate.m_clipping, L"clipped somewhere!");
			Assert::AreEqual(46720, opener_ms.constLastState().m_total_duration + opener_ms.constLastState().m_caststate.m_gcd, L"running duration is off");
            // infusion wears off AFTER fleche

            MoveStates_t movestates(opener_ms, -14900);

            movestates.advanceAndAddMove(end);

            Assert::AreEqual(31820, movestates.constLastState().m_total_duration, L"total duration is off");
            Assert::AreEqual(8142, movestates.constLastState().m_total_potency);
            AssertIsEssentuallyEqual(8142.0 / 31.82, movestates.constLastState().getDPS().calc());
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
			std::shared_ptr<Move_t> scorch(new Scorch_t());

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

			movestates.advanceAndAddMove(scorch);

            movestates.advanceAndAddMove(jolt2);

            movestates.advanceAndAddMove(end);

            return movestates;
        }

		TEST_METHOD(TryToAddMoveTest) {
			State_t state;
			state.m_statics.m_gcd = 2420;

			MoveStates_t movestates(state);

			std::shared_ptr<Move_t> jolt2(new Jolt2_t());
			Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)movestates.tryToAddMove(jolt2), L"Couldn't cast first Jolt 2");
			for (int i = 0; i < 2420; i += 10) {
				Assert::AreEqual((int)State_t::UA_NOT_USEABLE, (int)movestates.tryToAddMove(jolt2), L"Shouldn't be able to spam Jolt 2");
				movestates.advance(10);
			}
			Assert::AreEqual((int)State_t::UA_SUCCESS, (int)movestates.tryToAddMove(jolt2), L"Couldn't dualcast second Jolt 2");
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
			Assert::AreEqual(std::string("S"), it++->first->getChar());
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
            pot += 440; // L
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 470; // Re
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 200; // v
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 600; // B
            Assert::AreEqual(pot, it++->second.m_total_potency);
			pot += 700; // S
			Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 0; // J is still casting, no potency increase yet!
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 280; // END
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
            duration += 2420; // S, B done
            Assert::AreEqual(duration, it++->second.m_total_duration);
			duration += 2420; // J, S done
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
			Assert::AreEqual(std::string("S"), it++->first->getChar());
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
            pot += 440; // L
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 470; // Re
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 200; // v
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 600; // B
            Assert::AreEqual(pot, it++->second.m_total_potency);
			pot += 700; // S
			Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 0; // J is still casting, no potency increase yet!
            Assert::AreEqual(pot, it++->second.m_total_potency);
            pot += 280; // END
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
            duration += 2420; // S, B done
            Assert::AreEqual(duration, it++->second.m_total_duration);
			duration += 2420; // J, S done
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