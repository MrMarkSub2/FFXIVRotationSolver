#include "stdafx.h"
#include "CppUnitTest.h"
#include "utils.h"
#include "rdm_state.h"
#include "rdm_moves.h"
#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestCases
{
    TEST_CLASS(StateTests)
    {
    public:
        TEST_METHOD(CastMagicMissiles)
        {
            State_t state;
            state.m_statics.m_gcd = 2500;
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());

            Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)state.UseAction(jolt2));
            state.AdvanceTimeForGCD();
            AssertIsEssentuallyEqual(96.0, state.getDPS().calc());
        }

        TEST_METHOD(Doublecast) {
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            State_t state;

            Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)state.UseAction(jolt2), L"jolt not casting");
            state.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(jolt2), L"jolt didn't dualcast");
            state.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)state.UseAction(jolt2), L"dualcast stuck");
            state.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(jolt2), L"jolt didn't dualcast");
            state.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)state.UseAction(jolt2), L"dualcast stuck");
        }

        TEST_METHOD(WeaveFleche)
        {
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> fleche(new Fleche_t());

            State_t state1;
            state1.m_statics.m_gcd = 2500;
            Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)state1.UseAction(jolt2), L"state1 jolt not casting");
            state1.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state1.UseAction(verthunder), L"state1 verthunder not successful");
            state1.AdvanceTimeForGCD();
            AssertIsEssentuallyEqual(108.0, state1.getDPS().calc());

            State_t state2;
            state2.m_statics.m_gcd = 2500;
            Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)state2.UseAction(jolt2), L"state2 jolt not casting");
            state2.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state2.UseAction(verthunder), L"state2 verthunder not successful");
            Assert::AreEqual((int)State_t::UA_NOT_USEABLE, (int)state2.UseAction(fleche), L"state2 fleche immediately useable?!");
            state2.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state2.UseAction(fleche), L"state2 fleche not useable");
            state2.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_NOT_USEABLE, (int)state2.UseAction(fleche), L"state2 fleche usable immediately");
            AssertIsEssentuallyEqual(192.0, state2.getDPS().calc());
        }

        TEST_METHOD(VerflareIsPrettier) {
            State_t state;
            state.m_mana.add(80, 80);
            state.m_caststate.m_out_of_range = 0;
            state.m_statics.m_gcd = 2500;

            std::shared_ptr<Move_t> rip(new EnhRiposte_t());
            std::shared_ptr<Move_t> zwer(new EnhZwerchhau_t());
            std::shared_ptr<Move_t> redoub(new EnhRedoublement_t());
            std::shared_ptr<Move_t> verflare(new Verflare_t());

            std::shared_ptr<Move_t> fleche(new Fleche_t());
            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> disp(new Displacement_t());

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(rip), L"riposte failed");
            Assert::AreEqual(1500, state.m_caststate.m_gcd);
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(corps), L"corps failed");
            Assert::AreEqual(800, state.m_caststate.m_gcd);
            state.AdvanceTimeForGCD();

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(zwer), L"zwer failed");
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(fleche), L"fleche failed");
            state.AdvanceTimeForGCD();

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(redoub), L"redoub failed");
            Assert::AreEqual(2200, state.m_caststate.m_gcd);
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(disp), L"disp failed");
            state.AdvanceTimeForGCD();

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(verflare), L"verflare failed");
            state.AdvanceTimeForGCD();

            Assert::AreEqual(2200, state.m_total_potency);
            Assert::AreEqual(7700, state.m_total_duration);
            AssertIsEssentuallyEqual(2200.0 / 7.7, state.getDPS().calc());
        }

        TEST_METHOD(BOLDVerflare) {
            State_t state;
            state.m_mana.add(80, 80);
            state.m_caststate.m_out_of_range = 0;
            state.m_statics.m_gcd = 2500;

            std::shared_ptr<Move_t> rip(new EnhRiposte_t());
            std::shared_ptr<Move_t> zwer(new EnhZwerchhau_t());
            std::shared_ptr<Move_t> redoub(new EnhRedoublement_t());
            std::shared_ptr<Move_t> verflare(new Verflare_t());

            std::shared_ptr<Move_t> fleche(new Fleche_t());
            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> disp(new Displacement_t());

            std::shared_ptr<Move_t> embolden(new Embolden_t());

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(embolden), L"embolden failed");
            state.AdvanceTimeForAbility();

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(rip), L"riposte failed");
            Assert::AreEqual(1500, state.m_caststate.m_gcd);
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(corps), L"corps failed");
            Assert::AreEqual(800, state.m_caststate.m_gcd);
            state.AdvanceTimeForGCD();

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(zwer), L"zwer failed");
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(fleche), L"fleche failed");
            state.AdvanceTimeForGCD();

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(redoub), L"redoub failed");
            Assert::AreEqual(2200, state.m_caststate.m_gcd);
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(disp), L"disp failed");
            state.AdvanceTimeForGCD();

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(verflare), L"verflare failed");
            state.AdvanceTimeForGCD();

            Assert::AreEqual(2341, state.m_total_potency);
            Assert::AreEqual(8400, state.m_total_duration);
            AssertIsEssentuallyEqual(2341.0 / 8.4, state.getDPS().calc());
        }

        TEST_METHOD(FlecheFleche) {
            State_t state;
            std::shared_ptr<Move_t> fleche(new Fleche_t());

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(fleche), L"fleche #1 failed");
            state.AdvanceTimeByMilliseconds(fleche->getRecast());
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(fleche), L"fleche #2 failed");
        }

        TEST_METHOD(HoHum) {
            State_t state;
            state.m_statics.m_gcd = 2420;
            state.m_statics.m_test_duration = 15 * 60 * 1000;

            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());

            for (int spellnum = 0; state.m_total_duration < state.m_statics.m_test_duration; ++spellnum) {
                if (spellnum % 2 == 0)
                    Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)state.UseAction(jolt2), convert_to_wc((std::string("jolt not casting ")+std::to_string(spellnum)).c_str()));
                else
                    Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(verthunder), convert_to_wc((std::string("verthunder not successful ") + std::to_string(spellnum)).c_str()));

                state.AdvanceTimeForGCD();
            }

            Assert::AreEqual(100440, state.m_total_potency);
            Assert::AreEqual(15*60*1000, state.m_total_duration);
            AssertIsEssentuallyEqual(111.6, state.getDPS().calc());
        }

        TEST_METHOD(ManificationTest) {
            State_t state;
            state.m_statics.m_gcd = 2500;
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> disp(new Displacement_t());
            std::shared_ptr<Move_t> mani(new Manification_t());

            state.UseAction(jolt2);
            state.AdvanceTimeForGCD();
            state.UseAction(verthunder);
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(corps), L"corps #1 not successful");
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(disp), L"disp #1 not successful");
            state.AdvanceTimeForAbility();
            Assert::AreEqual(2500 + 700 + 700 + 1350, state.m_total_duration);
            state.UseAction(mani);
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(corps), L"corps #2 not successful");
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(disp), L"disp #2 not successful");
            state.AdvanceTimeForAbility();
            Assert::AreEqual(2500 + 700 + 700 + 1350 + 700 + 700 + 1350, state.m_total_duration);

            Assert::AreEqual(6, state.m_mana.white());
            Assert::AreEqual(28, state.m_mana.black());
        }

        TEST_METHOD(AccelerationTest) {
            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> accel(new Acceleration_t());

            // this simply can't fail 100 times in a row
            for (int i = 0; i < 100; ++i) {
                State_t state;
                
                Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(accel));
                state.AdvanceTimeForAbility();
                state.UseAction(jolt2);
                state.AdvanceTimeForGCD();
                state.UseAction(verthunder);

                Assert::IsTrue(state.m_status.m_verfire > 0);
            }
        }

        TEST_METHOD(CorpsDispTest) {
            State_t state;
            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> disp(new Displacement_t());
            std::shared_ptr<Move_t> rip(new Riposte_t());

            Assert::IsFalse(rip->IsUseable(state));
            state.UseAction(corps);
            state.AdvanceTimeForAbility();
            Assert::IsTrue(rip->IsUseable(state));
            state.UseAction(disp);
            state.AdvanceTimeForAbility();
            Assert::IsFalse(rip->IsUseable(state));
        }

        TEST_METHOD(SwiftcastTest) {
            State_t state;
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> swift(new Swiftcast_t());

            state.UseAction(swift);
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(verthunder));
        }

        TEST_METHOD(AoETest) {
            State_t state;
            std::shared_ptr<Move_t> moul(new EnhMoulinet_t());
            std::shared_ptr<Move_t> scatter(new Scatter_t());
            std::shared_ptr<Move_t> contre(new Contre_t());
            state.m_mana.add(30, 30);
            state.m_statics.m_targets = 10;
            state.m_caststate.m_out_of_range = 0;

            Assert::AreEqual((int)State_t::UA_IS_CASTING, (int)state.UseAction(scatter), L"scatter failed");
            state.AdvanceTimeForGCD();
            Assert::AreEqual(1000, state.m_total_potency);
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(moul), L"moul failed");
            Assert::AreEqual(1000 + 2000, state.m_total_potency);
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(contre), L"contre failed");
            Assert::AreEqual(1000 + 2000 + 1950, state.m_total_potency);
        }

        TEST_METHOD(GTFOManalessMelee) {
            State_t state;
            state.m_caststate.m_out_of_range = 0;

            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> rip(new Riposte_t());
            std::shared_ptr<Move_t> zwer(new Zwerchhau_t());
            std::shared_ptr<Move_t> redoub(new Redoublement_t());
            std::shared_ptr<Move_t> verflare(new Verflare_t());

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(corps), L"corps failed");
            state.AdvanceTimeForAbility();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(rip), L"riposte failed");
            state.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(zwer), L"zwer failed");
            state.AdvanceTimeForGCD();
            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(redoub), L"redoub failed");
            state.AdvanceTimeForGCD();
            Assert::IsFalse(verflare->IsUseable(state));
        }

        TEST_METHOD(InfusionTest) {
            State_t state;
            state.m_statics.m_gcd = 2500;

            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> infusion(new Infusion_t());

            Assert::AreEqual((int)State_t::UA_SUCCESS, (int)state.UseAction(infusion), L"infusion failed");
            state.AdvanceTimeForAbility();

            while (state.m_total_duration < 60 * 1000) {
                state.UseAction(jolt2);
                state.AdvanceTimeForGCD();
                state.UseAction(verthunder);
                state.AdvanceTimeForGCD();
            }

            Assert::AreEqual(540 * 6 + 569 * 6, state.m_total_potency);
        }

        TEST_METHOD(ClippingTest) {
            State_t state;
            state.m_statics.m_gcd = 2500;

            std::shared_ptr<Move_t> jolt2(new Jolt2_t());
            std::shared_ptr<Move_t> verthunder(new Verthunder_t());
            std::shared_ptr<Move_t> corps(new Corps_t());
            std::shared_ptr<Move_t> disp(new Displacement_t());

            state.UseAction(jolt2);
            state.AdvanceTimeForGCD();
            state.UseAction(verthunder);
            state.AdvanceTimeForAbility();
            state.UseAction(corps);
            state.AdvanceTimeForAbility();
            state.UseAction(disp);
            state.AdvanceTimeForAbility();
            state.UseAction(jolt2);
            state.AdvanceTimeForGCD();

            Assert::AreEqual(250, state.m_caststate.m_clipping);
        }
    };
}