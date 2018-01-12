#pragma once

#include "dps.h"
#include "mana.h"
#include <memory>

class Move_t;

#ifdef RDM_NESTROTATION_EXPORTS
#define RDM_NESTROTATION_API __declspec(dllexport)
#else
#define RDM_NESTROTATION_API __declspec(dllimport)
#endif

class RDM_NESTROTATION_API State_t {
public:
    State_t();

    struct RDM_NESTROTATION_API Statics_t {
        Statics_t();

        int m_gcd; // in ms, your recurring GCD based on gear/buffs
        double m_infusion_buff; // what % does infusion increase DPS? 1.05? 1.2?
        int m_test_duration; // in ms, how long does the encounter last, e.g. 180,000 for 3min
        int m_targets; // for optimizing trash packs
        int m_running_ms_per_yalm; // in ms, how fast can I run a yalm? Important for opener, Displacement
    } m_statics;

    int m_total_potency;
    int m_total_duration; // in ms
    Mana_t m_mana;
    
    struct RDM_NESTROTATION_API CastState_t {
        CastState_t();

        int m_gcd; // in ms, how long until the next GCD becomes available
        int m_casting; // in ms, how long until I'm no longer casting Jolt
        Move_t* m_casting_move;
        int m_animation_lock; // in ms, how long until I'm no longer anilocked casting Acceleration
        int m_out_of_range; // in ms, time to get back into melee range (while not casting)
        int m_clipping; // in ms, how long my GCD was clipped by using abilities
        int m_dead_time; // in ms, how long I'm standing around picking my nose
    } m_caststate;

    struct RDM_NESTROTATION_API Status_t {
        Status_t();

        int m_dualcast; // in ms
        int m_impactful; // in ms
        int m_verstone; // in ms
        int m_verfire; // in ms
        int m_scatter; // in ms
        int m_acceleration; // in ms
        int m_embolden; // in ms until wears off, > 16000 is V, > 12000 is IV, etc
        int m_swiftcast; // in ms
        int m_infusion; // in ms
    } m_status;

    struct RDM_NESTROTATION_API Recast_t {
        Recast_t();

        int m_corps; // in ms
        int m_displacement; // in ms
        int m_acceleration; // in ms
        int m_manification; // in ms
        int m_embolden; // in ms
        int m_swiftcast; // in ms
        int m_fleche; // in ms
        int m_contre; // in ms
        int m_infusion; // in ms
    } m_recast;

    enum RDM_NESTROTATION_API MeleeCombo_t {
        MELEE_RIPOSTE,
        MELEE_ZWER,
        MELEE_REDOUBLEMENT,
        MELEE_FINISHER
    } m_melee_combo;

    // gets the DPS
    DPS_t getDPS() const;

    // not casting, GCD not rolling, not in animation lock... just chillin'
    bool isStandingStill() const;

    // lets us know if we can even use this move or if we need to wait
    bool ActionUseable(std::shared_ptr<Move_t> move) const;

    enum UseActionRval_t {
        UA_NOT_USEABLE,
        UA_IS_CASTING,
        UA_SUCCESS
    };

    // main event loop processor
    UseActionRval_t UseAction(std::shared_ptr<Move_t> move);

    // these are just helpers for unit tests
    void AdvanceTimeForGCD();
    void AdvanceTimeForSpell();
    void AdvanceTimeForAbility();
    void AdvanceTimeByMilliseconds(int ms);

private:
    bool ActionUseable(const Move_t* move) const;

    double EmboldenModifier(bool is_magic_dmg) const;
    double InfusionModifier() const;

    bool CastAction(Move_t* move, bool hasCastTime = true);
    bool CastAction(bool hasCastTime = true);

};
