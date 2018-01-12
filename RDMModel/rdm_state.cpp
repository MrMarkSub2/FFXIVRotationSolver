#include "stdafx.h"
#include "rdm_moves.h"
#include "rdm_state.h"
#include "utils.h"
#include <memory>

namespace {
    // GCD calculations
    const int g_ss = 849;
    const double g_huton = 0.0; /* { 0.0, 0.05, 0.1, 0.15, 0.2 } */
    const double g_haste = 0.0; /* { -0.2, -0.15, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3 } */
    const double g_arrow = 0.0; /* { 0.0, 0.05, 0.1, 0.15 } */
    const double g_feywind = 0.0; /* { 0.0, 0.03 } */
    const double g_UI3AF3 = 1.0; /* { 0.5, 1.0, 1.15 } */

    // infusion calculations
    /*const int g_base_int = 2541;
    const double g_infusion_boost = 0.1; // HQ Infusion
    const int g_infusion_cap = 137; // HQ Infusion*/

    // Orinx's Gear Comparison Sheet, BiS 340 + Genji + Persimmon, INT 2541
    const double g_infusion_buff = 1.0544957027472286;

    const int g_test_duration = 5 * 60 * 1000; // in ms

    // https://www.reddit.com/r/ffxiv/comments/3ccchg/til_the_squares_on_the_map_are_50_yalms_wide/csvrege/
    const int g_running_ms_per_yalm = 6000 / 50;
}

State_t::State_t()
    : m_total_potency(0), m_total_duration(0), m_mana(0,0), m_melee_combo(MELEE_RIPOSTE)
{ }

State_t::Statics_t::Statics_t()
    : m_gcd(calculateGCD(g_ss, g_huton, g_haste, g_arrow, g_feywind, g_UI3AF3)),
      m_infusion_buff(g_infusion_buff),
      m_test_duration(g_test_duration),
      m_targets(1),
      m_running_ms_per_yalm(g_running_ms_per_yalm)
{ }

State_t::CastState_t::CastState_t()
    : m_gcd(0), 
      m_casting(0), 
      m_casting_move(0), 
      m_animation_lock(0), 
      m_out_of_range(25 * g_running_ms_per_yalm), 
      m_clipping(0),
      m_dead_time(0)
{ }

State_t::Status_t::Status_t()
    : m_dualcast(0), 
      m_impactful(0), 
      m_verstone(0),
      m_verfire(0),
      m_acceleration(0),
      m_embolden(0),
      m_swiftcast(0),
      m_infusion(0),
      m_scatter(0)
{ }

State_t::Recast_t::Recast_t()
    : m_corps(0),
      m_displacement(0),
      m_acceleration(0),
      m_manification(0),
      m_embolden(0),
      m_swiftcast(0),
      m_fleche(0),
      m_contre(0),
      m_infusion(0)
{ }

DPS_t State_t::getDPS() const
{
    return DPS_t(m_total_potency, m_total_duration);
}

bool State_t::ActionUseable(const Move_t* move) const {
    if (!move)
        return false;

    if (m_caststate.m_animation_lock > 0)
        return false;

    if (m_caststate.m_casting > 0)
        return false;

    if ((move->getType() != Move_t::ABILITY_TYPE) && (m_caststate.m_gcd > 0))
        return false;

    return move->IsUseable(*this);
}

bool State_t::ActionUseable(std::shared_ptr<Move_t> move) const {
    return ActionUseable(move.get());
}

double State_t::EmboldenModifier(bool is_magic_dmg) const {
    if (!is_magic_dmg)
        return 1.0;

    int emboldenTime = m_status.m_embolden;

    if (emboldenTime > 16 * 1000)
        return 1.1;
    else if (emboldenTime > 12 * 1000)
        return 1.08;
    else if (emboldenTime > 8 * 1000)
        return 1.06;
    else if (emboldenTime > 4 * 1000)
        return 1.04;
    else if (emboldenTime > 0)
        return 1.02;
    else
        return 1.0;
}

double State_t::InfusionModifier() const {
    return m_status.m_infusion > 0 ? m_statics.m_infusion_buff : 1.0;
}

bool State_t::CastAction(Move_t* move, bool hasCastTime) {
    if (!move)
        return false;

    if (!ActionUseable(move))
        return false;

    m_total_potency += (int)(move->CalculatePotency(*this) * EmboldenModifier(move->IsMagicalDamage()) * InfusionModifier());
    move->Execute(*this);
    m_mana.add(move->getWhite(), move->getBlack());

    if (move->getType() == Move_t::SPELL_TYPE && hasCastTime)
        m_status.m_dualcast = 15 * 1000;

    return true;
}

bool State_t::CastAction(bool hasCastTime) {
    if (CastAction(m_caststate.m_casting_move, hasCastTime)) {
        m_caststate.m_casting_move = 0;
        return true;
    }

    return false;
}

State_t::UseActionRval_t State_t::UseAction(std::shared_ptr<Move_t> move)
{
    if (!ActionUseable(move))
        return UA_NOT_USEABLE;

    int castTime = (m_status.m_dualcast > 0 || m_status.m_swiftcast > 0) ? 0 : move->getCast() * m_statics.m_gcd / 25000 * 10; // rounds to nearest 10ms

    if (move->getType() != Move_t::ABILITY_TYPE) {
        if (move->getType() == Move_t::SPELL_TYPE && m_status.m_swiftcast > 0)
            m_status.m_swiftcast = 0;
        else
            m_status.m_dualcast = 0;

        m_caststate.m_casting = castTime;
        m_caststate.m_casting_move = move.get();
        bool rval = CastAction(castTime > 0);
        if (move->getType() == Move_t::WEAPONSKILL_TYPE)
            m_caststate.m_gcd = move->getRecast();
        else
            m_caststate.m_gcd = move->getRecast() * m_statics.m_gcd / 25000 * 10; // rounds to nearest 10ms
        m_caststate.m_animation_lock = move->getAnimationLock(); // not affected by SS

        return rval ? UA_SUCCESS : UA_IS_CASTING;
    }
    else {
        if (!CastAction(move.get(), castTime > 0))
            return UA_NOT_USEABLE;

        const int recast = move->getRecast(); // not affected by SS
        const std::string movechar = move->getChar();

        //TODO: optimize with map<string, int&>
        if (movechar == "^")
            m_recast.m_corps = recast;
        else if (movechar == "v")
            m_recast.m_displacement = recast;
        else if (movechar == "@")
            m_recast.m_acceleration = recast;
        else if (movechar == "2")
            m_recast.m_manification = recast;
        else if (movechar == "+")
            m_recast.m_embolden = recast;
        else if (movechar == "!")
            m_recast.m_swiftcast = recast;
        else if (movechar == "L")
            m_recast.m_fleche = recast;
        else if (movechar == "6")
            m_recast.m_contre = recast;
        else if (movechar == "I")
            m_recast.m_infusion = recast;
        else if (movechar != "_")
            throw std::runtime_error(std::string("Unexpected ability getChar()=") + movechar);

        m_caststate.m_animation_lock = move->getAnimationLock();

        return UA_SUCCESS;
    }
}

void State_t::AdvanceTimeForGCD() {
    AdvanceTimeByMilliseconds(m_caststate.m_gcd);
}

void State_t::AdvanceTimeForSpell() {
    AdvanceTimeByMilliseconds(m_caststate.m_casting);
}

void State_t::AdvanceTimeForAbility() {
    AdvanceTimeByMilliseconds(m_caststate.m_animation_lock);
}

void advanceHelper(int& num, int ms) {
    num = max(num + ms, 0);
}

void State_t::AdvanceTimeByMilliseconds(int ms) {
    for (; ms > 0; ms -= 10) {
        if (m_total_duration >= m_statics.m_test_duration)
            return;

        // safeguard in case of opener starting at -10 seconds. Don't round negative up to 0.
        m_total_duration += 10;

        if (m_caststate.m_gcd == 0 && m_caststate.m_animation_lock > 0)
            m_caststate.m_clipping += 10;

        if (m_caststate.m_gcd == 0 && m_caststate.m_casting == 0 && m_caststate.m_animation_lock == 0)
            m_caststate.m_dead_time += 10;

        advanceHelper(m_caststate.m_gcd, -10);
        advanceHelper(m_caststate.m_casting, -10);
        advanceHelper(m_caststate.m_animation_lock, -10);
        advanceHelper(m_caststate.m_out_of_range, -10);

        advanceHelper(m_status.m_dualcast, -10);
        advanceHelper(m_status.m_impactful, -10);
        advanceHelper(m_status.m_verstone, -10);
        advanceHelper(m_status.m_verfire, -10);
        advanceHelper(m_status.m_scatter, -10);
        advanceHelper(m_status.m_acceleration, -10);
        advanceHelper(m_status.m_embolden, -10);
        advanceHelper(m_status.m_swiftcast, -10);
        advanceHelper(m_status.m_infusion, -10);

        advanceHelper(m_recast.m_corps, -10);
        advanceHelper(m_recast.m_displacement, -10);
        advanceHelper(m_recast.m_acceleration, -10);
        advanceHelper(m_recast.m_manification, -10);
        advanceHelper(m_recast.m_embolden, -10);
        advanceHelper(m_recast.m_swiftcast, -10);
        advanceHelper(m_recast.m_fleche, -10);
        advanceHelper(m_recast.m_contre, -10);
        advanceHelper(m_recast.m_infusion, -10);

        CastAction();
    }
}