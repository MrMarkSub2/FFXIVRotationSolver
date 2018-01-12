#include "stdafx.h"
#include "moves.h"

struct Move_t::Impl {
    Impl() {
        m_name = "Undefined";
        m_char = "?";
        m_type = SPELL_TYPE;
        m_is_magical_damage = true;
        m_cast = 2000;
        m_recast = 2500;
        m_white = 0;
        m_black = 0;
        m_potency = 0;
        m_combo_potency = 0;
        m_animation_lock = 700;
    }

    std::string m_name;
    std::string m_char;
    Type_t m_type;
    bool m_is_magical_damage;
    int m_cast; // in ms
    int m_recast; // in ms
    int m_white;
    int m_black;
    int m_potency;
    int m_combo_potency;
    int m_animation_lock; // in ms
};

Move_t::Move_t() : pimpl(new Impl) { }
Move_t::Move_t(const Move_t & m) : pimpl(new Impl(*m.pimpl)) { }
Move_t& Move_t::operator=(const Move_t& m) {
    if (this != &m)
        pimpl = new Impl(*m.pimpl);
    
    return *this;
}
Move_t::~Move_t() { delete pimpl; }

void Move_t::Execute(State_t & state) const
{
}

bool Move_t::IsUseable(const State_t & state) const
{
    return true;
}

int Move_t::CalculatePotency(const State_t & state) const
{
    return pimpl->m_potency;
}

const std::string& Move_t::getName() const { return pimpl->m_name; }
const std::string& Move_t::getChar() const { return pimpl->m_char; }
Move_t::Type_t Move_t::getType() const { return pimpl->m_type; }
bool Move_t::IsMagicalDamage() const { return pimpl->m_is_magical_damage; }
int Move_t::getCast() const { return pimpl->m_cast; }
int Move_t::getRecast() const { return pimpl->m_recast; }
int Move_t::getWhite() const { return pimpl->m_white; }
int Move_t::getBlack() const { return pimpl->m_black; }
//int Move_t::getPotency() const { return pimpl->m_potency; }
//int Move_t::getComboPotency() const { return pimpl->m_combo_potency; }
int Move_t::getAnimationLock() const { return pimpl->m_animation_lock; }

struct MoveString_t::Impl {
    container_t m_moves;
};

MoveString_t::MoveString_t() : pimpl(new Impl) { }
MoveString_t::MoveString_t(const MoveString_t & ms) : pimpl(new Impl(*ms.pimpl)) { }
MoveString_t & MoveString_t::operator=(const MoveString_t& ms)
{
    if (this != &ms)
        pimpl = new Impl(*ms.pimpl);

    return *this;
}
MoveString_t::~MoveString_t() { delete pimpl; }

MoveString_t& MoveString_t::addMove(std::shared_ptr<Move_t> move) {
    pimpl->m_moves.push_back(move);

    return *this;
}

MoveString_t::container_t::const_iterator MoveString_t::begin() const { return pimpl->m_moves.begin(); }
MoveString_t::container_t::const_iterator MoveString_t::end() const { return pimpl->m_moves.end(); }

struct MoveStates_t::Impl {
    container_t m_moves;
    State_t m_last_state;
    std::shared_ptr<Move_t> m_last_move;
};

MoveStates_t::MoveStates_t(State_t& state) : pimpl(new Impl) {
    pimpl->m_last_state = state;
}
MoveStates_t::MoveStates_t(const MoveStates_t & opener, int offset)
    : pimpl(new Impl(*opener.pimpl))
{
    for (container_t::iterator it = begin(); it != end(); ++it) {
        it->second.m_total_duration += offset;
    }

    pimpl->m_last_state.m_total_duration += offset;
}
MoveStates_t::MoveStates_t(const MoveStates_t & ms) : pimpl(new Impl(*ms.pimpl)) { }
MoveStates_t & MoveStates_t::operator=(const MoveStates_t& ms)
{
    if (this != &ms)
        pimpl = new Impl(*ms.pimpl);

    return *this;
}
MoveStates_t::~MoveStates_t() { delete pimpl; }

MoveStates_t& MoveStates_t::advanceAndAddMove(std::shared_ptr<Move_t> move) {
    while (!isFinished() && pimpl->m_last_state.UseAction(move) == State_t::UA_NOT_USEABLE)
        pimpl->m_last_state.AdvanceTimeByMilliseconds(10);

    if (!isFinished())
        pimpl->m_moves.push_back(make_pair(move, pimpl->m_last_state));

    pimpl->m_last_move = move;

    return *this;
}

MoveStates_t & MoveStates_t::advanceAndAddMoveString(const MoveString_t & movestring)
{
    for (MoveString_t::container_t::const_iterator it = movestring.begin(); it != movestring.end(); ++it) {
        while (!isFinished() && pimpl->m_last_state.UseAction(*it) == State_t::UA_NOT_USEABLE)
            pimpl->m_last_state.AdvanceTimeByMilliseconds(10);

        if (!isFinished())
            pimpl->m_moves.push_back(make_pair(*it, pimpl->m_last_state));

        pimpl->m_last_move = *it;
    }

    return *this;
}

MoveStates_t & MoveStates_t::advance(int ms)
{
    pimpl->m_last_state.AdvanceTimeByMilliseconds(ms);
    return *this;
}

const State_t & MoveStates_t::constLastState() const
{
    return pimpl->m_last_state;
}

void MoveStates_t::finish(int total_ms)
{
    // like addMove, but we allow this to be added at the very very end of the time duration
    std::shared_ptr<Move_t> move(new EndPlaceholder_t());
    if (total_ms == -1 || total_ms < pimpl->m_last_state.m_total_duration) {
        while (!isFinished() && pimpl->m_last_state.UseAction(move) == State_t::UA_NOT_USEABLE)
            pimpl->m_last_state.AdvanceTimeByMilliseconds(10);
    }
    else {
        pimpl->m_last_state.AdvanceTimeByMilliseconds(total_ms - pimpl->m_last_state.m_total_duration);
    }

    pimpl->m_moves.push_back(make_pair(move, pimpl->m_last_state));
    pimpl->m_last_move = move;
}

bool MoveStates_t::isFinished() const
{
    return (pimpl->m_last_state.m_total_duration >= pimpl->m_last_state.m_statics.m_test_duration ||
        (pimpl->m_last_move && pimpl->m_last_move->getChar() == "END"));
}

void MoveStates_t::print(std::ostream & os, MS_PRINT_MODE_t printmode, char delim, int length) const
{
    int len = 0;
    for (MoveStates_t::container_t::const_iterator it = begin(); it != end() && (length == -1 || len < length); ++it, ++len) {
        size_t minnamelen = 8;
        switch (printmode) {
        case MS_PRINT_DPS:     
            os << (double)it->second.m_total_duration / 1000 << delim;
            if (it->first->getType() == Move_t::ABILITY_TYPE) {
                os << '[' << it->first->getName() << ']';
                minnamelen -= 2;
            }
            else {
                os << it->first->getName();
            }
            if (it->first->getType() == Move_t::SPELL_TYPE && it->second.m_caststate.m_casting == 0) {
                os << " *";
                minnamelen -= 2;
            }
            os << delim;
            if (delim == '\t' && it->first->getName().length() < minnamelen)
                os << delim;
            os << it->second.getDPS();
            os << std::endl;
            break;
        case MS_PRINT_CHARLIST:
            if (it->first->getType() == Move_t::ABILITY_TYPE)
                os << '[' << it->first->getChar() << ']' << delim;
            else
                os << it->first->getChar() << delim;
            break;
        }
    }

    if (printmode == MS_PRINT_DPS) {
        os << std::endl;
        os << "Clipping time: " << (double)pimpl->m_last_state.m_caststate.m_clipping / 1000 << std::endl;
        os << "Dead time: " << (double)pimpl->m_last_state.m_caststate.m_dead_time / 1000 << std::endl;
        os << std::endl;
    }

    if (printmode == MS_PRINT_CHARLIST)
        os << std::endl;
}

MoveStates_t::container_t::iterator MoveStates_t::begin() { return pimpl->m_moves.begin(); }
MoveStates_t::container_t::iterator MoveStates_t::end() { return pimpl->m_moves.end(); }
MoveStates_t::container_t::const_iterator MoveStates_t::begin() const { return pimpl->m_moves.begin(); }
MoveStates_t::container_t::const_iterator MoveStates_t::end() const { return pimpl->m_moves.end(); }

void swap(Move_t & m1, Move_t & m2)
{
    std::swap(m1.pimpl, m2.pimpl);
}

void swap(MoveString_t & ms1, MoveString_t & ms2)
{
    std::swap(ms1.pimpl, ms2.pimpl);
}

void swap(MoveStates_t & ms1, MoveStates_t & ms2)
{
    std::swap(ms1.pimpl, ms2.pimpl);
}

MoveStates_t getMinimalisticOpener()
{
    State_t opener_state;
    opener_state.m_caststate.m_clipping = -1400; // offset allegedly clipping in precast accel/div
    opener_state.m_caststate.m_dead_time = -8600 - (-5000 * opener_state.m_statics.m_gcd / 25000 * 10); // offset allegedly dead time in precast accel/div
    MoveStates_t opener_ms(opener_state);

    std::shared_ptr<Move_t> accel(new Acceleration_t());
    std::shared_ptr<Move_t> div(new DiversionPlaceholder_t());
    std::shared_ptr<Move_t> veraero(new Veraero_t());

    opener_ms.advanceAndAddMove(accel);
    opener_ms.advance(4000);
    opener_ms.advanceAndAddMove(div);
    opener_ms.advance((-5000 * opener_state.m_statics.m_gcd / 25000 * 10) - (-6000)); // butt it right up against start time
    opener_ms.advanceAndAddMove(veraero);

    return MoveStates_t(opener_ms, -10 * 1000);
}

MoveStates_t getRaid5thGCDEmboldenOpener()
{
    State_t opener_state;
    opener_state.m_caststate.m_clipping = -1400; // offset allegedly clipping in precast accel/div
    opener_state.m_caststate.m_dead_time = -8600 - (-5000 * opener_state.m_statics.m_gcd / 25000 * 10); // offset allegedly dead time in precast accel/div
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

    opener_ms.advanceAndAddMove(accel);
    opener_ms.advance(4000);
    opener_ms.advanceAndAddMove(div);
    opener_ms.advance((-5000 * opener_state.m_statics.m_gcd / 25000 * 10) - (-6000)); // butt it right up against start time
    opener_ms.advanceAndAddMove(veraero); // 300 | 0
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
    opener_ms.advanceAndAddMove(disp); // 137 | 14.52 no clip
    opener_ms.advanceAndAddMove(corps); // 137 | 14.85 clipping!
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
    // infusion wears off AFTER fleche
    // embolden wears off AFTER embolden

    return MoveStates_t(opener_ms, -10 * 1000);
}

/*bool Move_t::Impl::operator==(const Move_t::Impl & rhs) const
{
    return ((m_procChance == rhs.m_procChance) &&
        (m_procType == rhs.m_procType) &&
        (m_pot == rhs.m_pot) &&
        (m_sec == rhs.m_sec) &&
        (m_white == rhs.m_white) &&
        (m_black == rhs.m_black) &&
        (m_name == rhs.m_name) &&
        (m_char == rhs.m_char) &&
        (m_isScript == rhs.m_isScript));
}*/

/*
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("J", "Jolt2", 240, gcd, 3, 3, 1.0, 'I')));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("I", "Impact", 270, gcd, 4, 4, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("F", "Verfire", 270, gcd, 0, 9, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("S", "Verstone", 270, gcd, 9, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("hA", "(hardcast)Veraero", 300, gcd*2, 11, 0, 0.5, 'S')));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("hT", "(hardcast)Verthunder", 300, gcd*2, 0, 11, 0.5, 'F')));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("A", "Veraero", 300, gcd, 11, 0, 0.5, 'S')));                 //dualcast
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("T", "Verthunder", 300, gcd, 0, 11, 0.5, 'F')));              //dualcast
MoveMap_Add(std::shared_ptr<Move_t>(createAccelerated(g_allMoves["A"], "@A", "AccelVeraero")));              //accelerated
MoveMap_Add(std::shared_ptr<Move_t>(createAccelerated(g_allMoves["T"], "@T", "AccelVerthunder")));           //accelerated
MoveMap_Add(std::shared_ptr<Move_t>(createSwiftcast(g_allMoves["hA"], "!A", "SwiftVeraero", gcd)));          //swiftcast
MoveMap_Add(std::shared_ptr<Move_t>(createSwiftcast(g_allMoves["hT"], "!T", "SwiftVerthunder", gcd)));       //swiftcast
MoveMap_Add(std::shared_ptr<Move_t>(createAccelerated(g_allMoves["!A"], "@!A", "AccelSwiftVeraero")));       //accelerated
MoveMap_Add(std::shared_ptr<Move_t>(createAccelerated(g_allMoves["!T"], "@!T", "AccelSwiftVerthunder")));    //accelerated

// Melee
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("Ri", "Riposte", 210, 1.5, -30, -30, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("Z", "Zwerchhau", 290, 1.5, -25, -25)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("Re", "Redoublement", 470, 2.2, -25, -25)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("W", "Verholy", 550, gcd, 21, 0, 0.2, 'S', true)));    //eval is smart enough to only use % if unbalanced
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("B", "Verflare", 550, gcd, 0, 21, 0.2, 'F', true)));   //eval is smart enough to only use % if unbalanced
MoveMap_Add(std::shared_ptr<Move_t>(createAccelerated(g_allMoves["W"], "@W", "AccelVerholy")));
MoveMap_Add(std::shared_ptr<Move_t>(createAccelerated(g_allMoves["B"], "@B", "AccelVerholy")));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("M", "Moulinet", 200, 1.5, -30, -30, 0.0, char(), true)));

// "Fun" conditional chains
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?IJ?", "?IJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?FJ?", "?FJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?SJ?", "?SJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?IFJ?", "?IFJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?ISJ?", "?ISJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?FIJ?", "?FIJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?FSJ?", "?FSJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?SIJ?", "?SIJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?SFJ?", "?SFJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?IFSJ?", "?IFSJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?ISFJ?", "?ISFJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?FISJ?", "?FISJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?FSIJ?", "?FSIJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?SIFJ?", "?SIFJ?", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("?SFIJ?", "?SFIJ?", 0, 0.0, 0, 0, 0.0, char(), true)));

// Other script moves
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("+", "Embolden", 0, 0.0, 0, 0, 0.0, char(), true)));
MoveMap_Add(std::shared_ptr<Move_t>(new Move_t("2", "Manification", 0, 0.0, 0, 0, 0.0, char(), true)));
*/

Jolt2_t::Jolt2_t()
{
    pimpl->m_name = "Jolt II";
    pimpl->m_char = "J";
    pimpl->m_type = SPELL_TYPE;
    pimpl->m_potency = 240;
    pimpl->m_white = 3;
    pimpl->m_black = 3;
}

void Jolt2_t::Execute(State_t & state) const
{
    state.m_status.m_impactful = 30 * 1000;
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

Impact_t::Impact_t()
{
    pimpl->m_name = "Impact";
    pimpl->m_char = "I";
    pimpl->m_type = SPELL_TYPE;
    pimpl->m_potency = 270;
    pimpl->m_white = 4;
    pimpl->m_black = 4;
}

void Impact_t::Execute(State_t & state) const
{
    state.m_status.m_impactful = 0;
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool Impact_t::IsUseable(const State_t & state) const
{
    return (state.m_status.m_impactful > 0);
}

Verthunder_t::Verthunder_t()
{
    pimpl->m_name = "Verthunder";
    pimpl->m_char = "T";
    pimpl->m_type = SPELL_TYPE;
    pimpl->m_cast = 5000;
    pimpl->m_potency = 300;
    pimpl->m_white = 0;
    pimpl->m_black = 11;
}

void Verthunder_t::Execute(State_t & state) const
{
    if ((state.m_status.m_acceleration > 0) || (rand() % 100 < 50))
        state.m_status.m_verfire = 30 * 1000;

    state.m_status.m_acceleration = 0;
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

Veraero_t::Veraero_t()
{
    pimpl->m_name = "Veraero";
    pimpl->m_char = "A";
    pimpl->m_type = SPELL_TYPE;
    pimpl->m_cast = 5000;
    pimpl->m_potency = 300;
    pimpl->m_white = 11;
    pimpl->m_black = 0;
}

void Veraero_t::Execute(State_t & state) const
{
    if ((state.m_status.m_acceleration > 0) || (rand() % 100 < 50))
        state.m_status.m_verstone = 30 * 1000;

    state.m_status.m_acceleration = 0;
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

Verfire_t::Verfire_t()
{
    pimpl->m_name = "Verfire";
    pimpl->m_char = "F";
    pimpl->m_type = SPELL_TYPE;
    pimpl->m_potency = 270;
    pimpl->m_white = 0;
    pimpl->m_black = 9;
}

void Verfire_t::Execute(State_t & state) const
{
    state.m_status.m_verfire = 0;
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool Verfire_t::IsUseable(const State_t & state) const
{
    return (state.m_status.m_verfire > 0);
}

Verstone_t::Verstone_t()
{
    pimpl->m_name = "Verstone";
    pimpl->m_char = "S";
    pimpl->m_type = SPELL_TYPE;
    pimpl->m_potency = 270;
    pimpl->m_white = 9;
    pimpl->m_black = 0;
}

void Verstone_t::Execute(State_t & state) const
{
    state.m_status.m_verstone = 0;
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool Verstone_t::IsUseable(const State_t & state) const
{
    return (state.m_status.m_verstone > 0);
}

Verflare_t::Verflare_t()
{
    pimpl->m_name = "Verflare";
    pimpl->m_char = "B";
    pimpl->m_type = FINISHER_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_potency = 0;
    pimpl->m_combo_potency = 550;
    pimpl->m_white = 0;
    pimpl->m_black = 21;
}

void Verflare_t::Execute(State_t & state) const
{
    state.m_melee_combo = State_t::MELEE_RIPOSTE;

    if (state.m_mana.black() < state.m_mana.white()) {
        state.m_status.m_verfire = 30 * 1000;
        return; // acceleration is protected in this case
    }

    if ((state.m_status.m_acceleration > 0) || (rand() % 100 < 20))
        state.m_status.m_verfire = 30 * 1000;

    state.m_status.m_acceleration = 0;
}

bool Verflare_t::IsUseable(const State_t & state) const
{
    return (state.m_melee_combo == State_t::MELEE_FINISHER);
}

int Verflare_t::CalculatePotency(const State_t & state) const
{
    return pimpl->m_combo_potency;
}

Verholy_t::Verholy_t()
{
    pimpl->m_name = "Verholy";
    pimpl->m_char = "H";
    pimpl->m_type = FINISHER_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_potency = 0;
    pimpl->m_combo_potency = 550;
    pimpl->m_white = 21;
    pimpl->m_black = 0;
}

void Verholy_t::Execute(State_t & state) const
{
    state.m_melee_combo = State_t::MELEE_RIPOSTE;

    if (state.m_mana.white() < state.m_mana.black()) {
        state.m_status.m_verstone = 30 * 1000;
        return; // acceleration is protected in this case
    }

    if ((state.m_status.m_acceleration > 0) || (rand() % 100 < 20))
        state.m_status.m_verstone = 30 * 1000;

    state.m_status.m_acceleration = 0;
}

bool Verholy_t::IsUseable(const State_t & state) const
{
    return (state.m_melee_combo == State_t::MELEE_FINISHER);
}

int Verholy_t::CalculatePotency(const State_t & state) const
{
    return pimpl->m_combo_potency;
}

Corps_t::Corps_t()
{
    pimpl->m_name = "Corps-a-corps";
    pimpl->m_char = "^";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_is_magical_damage = false;
    pimpl->m_cast = 0;
    pimpl->m_recast = 40 * 1000;
    pimpl->m_potency = 130;
}

void Corps_t::Execute(State_t & state) const
{
    state.m_caststate.m_out_of_range = 0;
    state.m_recast.m_corps = pimpl->m_recast;
}

bool Corps_t::IsUseable(const State_t & state) const
{
    return (state.m_recast.m_corps == 0);
}

Displacement_t::Displacement_t()
{
    pimpl->m_name = "Displacement";
    pimpl->m_char = "v";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_is_magical_damage = false;
    pimpl->m_cast = 0;
    pimpl->m_recast = 35 * 1000;
    pimpl->m_potency = 130;
    pimpl->m_animation_lock = 1350;
}

void Displacement_t::Execute(State_t & state) const
{
    state.m_caststate.m_out_of_range = 15 * state.m_statics.m_running_ms_per_yalm;
    state.m_recast.m_displacement = pimpl->m_recast;
}

bool Displacement_t::IsUseable(const State_t & state) const
{
    return ((state.m_recast.m_displacement == 0) && (state.m_caststate.m_out_of_range == 0));
}

Acceleration_t::Acceleration_t()
{
    pimpl->m_name = "Acceleration";
    pimpl->m_char = "@";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_recast = 35 * 1000;
}

void Acceleration_t::Execute(State_t & state) const
{
    state.m_status.m_acceleration = 10 * 1000;
    state.m_recast.m_acceleration = pimpl->m_recast;
}

bool Acceleration_t::IsUseable(const State_t & state) const
{
    return (state.m_recast.m_acceleration == 0);
}

Manification_t::Manification_t()
{
    pimpl->m_name = "Manification";
    pimpl->m_char = "2";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_recast = 120 * 1000;
}

void Manification_t::Execute(State_t & state) const
{
    state.m_mana.add(state.m_mana.white(), state.m_mana.black());
    state.m_recast.m_corps = 0;
    state.m_recast.m_displacement = 0;
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
    state.m_recast.m_manification = pimpl->m_recast;
}

bool Manification_t::IsUseable(const State_t & state) const
{
    return (state.m_recast.m_manification == 0);
}

Embolden_t::Embolden_t()
{
    pimpl->m_name = "Embolden";
    pimpl->m_char = "+";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_recast = 120 * 1000;
}

void Embolden_t::Execute(State_t & state) const
{
    state.m_status.m_embolden = 20 * 1000;
    state.m_recast.m_embolden = pimpl->m_recast;
}

bool Embolden_t::IsUseable(const State_t & state) const
{
    return (state.m_recast.m_embolden == 0);
}

Swiftcast_t::Swiftcast_t()
{
    pimpl->m_name = "Swiftcast";
    pimpl->m_char = "!";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_recast = 60 * 1000;
}

void Swiftcast_t::Execute(State_t & state) const
{
    state.m_status.m_swiftcast = 10 * 1000;
    state.m_recast.m_swiftcast = pimpl->m_recast;
}

bool Swiftcast_t::IsUseable(const State_t & state) const
{
    return (state.m_recast.m_swiftcast == 0);
}

Fleche_t::Fleche_t()
{
    pimpl->m_name = "Fletch";
    pimpl->m_char = "L";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_is_magical_damage = false;
    pimpl->m_cast = 0;
    pimpl->m_recast = 25 * 1000;
    pimpl->m_potency = 420;
}

void Fleche_t::Execute(State_t & state) const
{
    state.m_recast.m_fleche = pimpl->m_recast;
}

bool Fleche_t::IsUseable(const State_t & state) const
{
    return (state.m_recast.m_fleche == 0);
}

Contre_t::Contre_t()
{
    pimpl->m_name = "Contre Sixte";
    pimpl->m_char = "6";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_is_magical_damage = false;
    pimpl->m_cast = 0;
    pimpl->m_recast = 45 * 1000;
    pimpl->m_potency = 300;
}

void Contre_t::Execute(State_t & state) const
{
    state.m_recast.m_contre = pimpl->m_recast;
}

bool Contre_t::IsUseable(const State_t & state) const
{
    return (state.m_recast.m_contre == 0);
}

int Contre_t::CalculatePotency(const State_t & state) const
{
    int potency = 0;
    switch (state.m_statics.m_targets) {
    case 12:    potency += pimpl->m_potency * 5 /10;
    case 11:    potency += pimpl->m_potency * 5 / 10;
    case 10:    potency += pimpl->m_potency * 5 / 10;
    case 9:     potency += pimpl->m_potency * 5 / 10;
    case 8:     potency += pimpl->m_potency * 5 / 10;
    case 7:     potency += pimpl->m_potency * 5 / 10;
    case 6:     potency += pimpl->m_potency * 5 / 10;
    case 5:     potency += pimpl->m_potency * 6 / 10;
    case 4:     potency += pimpl->m_potency * 7 / 10;
    case 3:     potency += pimpl->m_potency * 8 / 10;
    case 2:     potency += pimpl->m_potency * 9 / 10;
    case 1:     potency += pimpl->m_potency;
        break;
    default:    throw std::runtime_error(std::string("Contre Sixte can't handle m_targets=") + std::to_string(state.m_statics.m_targets));
        break;
    }
    return potency;
}

Riposte_t::Riposte_t()
{
    pimpl->m_name = "NO MANA Riposte";
    pimpl->m_char = "Ri";
    pimpl->m_type = WEAPONSKILL_TYPE;
    pimpl->m_is_magical_damage = false;
    pimpl->m_cast = 0;
    pimpl->m_recast = 2500;
    pimpl->m_potency = 130;
}

void Riposte_t::Execute(State_t & state) const
{
    state.m_melee_combo = State_t::MELEE_ZWER;
}

bool Riposte_t::IsUseable(const State_t & state) const
{
    return ((state.m_caststate.m_out_of_range == 0) && 
        !(state.m_mana.white() >= 30 && state.m_mana.black() >= 30));
}

EnhRiposte_t::EnhRiposte_t()
{
    pimpl->m_name = "Riposte";
    pimpl->m_char = "Ri";
    pimpl->m_type = WEAPONSKILL_TYPE;
    pimpl->m_is_magical_damage = true;
    pimpl->m_cast = 0;
    pimpl->m_recast = 1500;
    pimpl->m_potency = 210;
    pimpl->m_white = -30;
    pimpl->m_black = -30;
}

void EnhRiposte_t::Execute(State_t & state) const
{
    state.m_melee_combo = State_t::MELEE_ZWER;
}

bool EnhRiposte_t::IsUseable(const State_t & state) const
{
    return ((state.m_caststate.m_out_of_range == 0) &&
        (state.m_mana.white() >= 30 && state.m_mana.black() >= 30));
}

Zwerchhau_t::Zwerchhau_t()
{
    pimpl->m_name = "NO MANA Zwerchhau";
    pimpl->m_char = "Z";
    pimpl->m_type = WEAPONSKILL_TYPE;
    pimpl->m_is_magical_damage = false;
    pimpl->m_cast = 0;
    pimpl->m_recast = 2500;
    pimpl->m_potency = 100;
    pimpl->m_combo_potency = 150;
}

void Zwerchhau_t::Execute(State_t & state) const
{
    if (state.m_melee_combo == State_t::MELEE_ZWER)
        state.m_melee_combo = State_t::MELEE_REDOUBLEMENT;
    else
        state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool Zwerchhau_t::IsUseable(const State_t & state) const
{
    return ((state.m_caststate.m_out_of_range == 0) &&
        !(state.m_mana.white() >= 25 && state.m_mana.black() >= 25));
}

int Zwerchhau_t::CalculatePotency(const State_t & state) const
{
    return state.m_melee_combo == State_t::MELEE_ZWER ? pimpl->m_combo_potency : pimpl->m_potency;
}

EnhZwerchhau_t::EnhZwerchhau_t()
{
    pimpl->m_name = "Zwerchhau";
    pimpl->m_char = "Z";
    pimpl->m_type = WEAPONSKILL_TYPE;
    pimpl->m_is_magical_damage = true;
    pimpl->m_cast = 0;
    pimpl->m_recast = 1500;
    pimpl->m_potency = 100;
    pimpl->m_combo_potency = 290;
    pimpl->m_white = -25;
    pimpl->m_black = -25;
}

void EnhZwerchhau_t::Execute(State_t & state) const
{
    if (state.m_melee_combo == State_t::MELEE_ZWER)
        state.m_melee_combo = State_t::MELEE_REDOUBLEMENT;
    else
        state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool EnhZwerchhau_t::IsUseable(const State_t & state) const
{
    return ((state.m_caststate.m_out_of_range == 0) &&
        (state.m_mana.white() >= 25 && state.m_mana.black() >= 25));
}

int EnhZwerchhau_t::CalculatePotency(const State_t & state) const
{
    return state.m_melee_combo == State_t::MELEE_ZWER ? pimpl->m_combo_potency : pimpl->m_potency;
}

Redoublement_t::Redoublement_t()
{
    pimpl->m_name = "NO MANA Redoublement";
    pimpl->m_char = "Re";
    pimpl->m_type = WEAPONSKILL_TYPE;
    pimpl->m_is_magical_damage = false;
    pimpl->m_cast = 0;
    pimpl->m_recast = 2500;
    pimpl->m_potency = 100;
    pimpl->m_combo_potency = 230;
}

void Redoublement_t::Execute(State_t & state) const
{
    // regular Redoublement cannot go to Verholy/Verflare, only Enhanced versions
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool Redoublement_t::IsUseable(const State_t & state) const
{
    return ((state.m_caststate.m_out_of_range == 0) &&
        !(state.m_mana.white() >= 25 && state.m_mana.black() >= 25));
}

int Redoublement_t::CalculatePotency(const State_t & state) const
{
    return state.m_melee_combo == State_t::MELEE_REDOUBLEMENT ? pimpl->m_combo_potency : pimpl->m_potency;
}

EnhRedoublement_t::EnhRedoublement_t()
{
    pimpl->m_name = "Redoublement";
    pimpl->m_char = "Re";
    pimpl->m_type = WEAPONSKILL_TYPE;
    pimpl->m_is_magical_damage = true;
    pimpl->m_cast = 0;
    pimpl->m_recast = 2200;
    pimpl->m_potency = 100;
    pimpl->m_combo_potency = 470;
    pimpl->m_white = -25;
    pimpl->m_black = -25;
}

void EnhRedoublement_t::Execute(State_t & state) const
{
    if (state.m_melee_combo == State_t::MELEE_REDOUBLEMENT)
        state.m_melee_combo = State_t::MELEE_FINISHER;
    else
        state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool EnhRedoublement_t::IsUseable(const State_t & state) const
{
    return ((state.m_caststate.m_out_of_range == 0) &&
        (state.m_mana.white() >= 25 && state.m_mana.black() >= 25));
}

int EnhRedoublement_t::CalculatePotency(const State_t & state) const
{
    return state.m_melee_combo == State_t::MELEE_REDOUBLEMENT ? pimpl->m_combo_potency : pimpl->m_potency;
}

Scatter_t::Scatter_t()
{
    pimpl->m_name = "Scatter";
    pimpl->m_char = "*";
    pimpl->m_type = SPELL_TYPE;
    pimpl->m_potency = 100;
    pimpl->m_white = 3;
    pimpl->m_black = 3;
}

void Scatter_t::Execute(State_t & state) const
{
    if (rand() % 100 < 25)
        state.m_status.m_scatter = 10 * 1000;

    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool Scatter_t::IsUseable(const State_t & state) const
{
    return (state.m_status.m_scatter == 0);
}

int Scatter_t::CalculatePotency(const State_t & state) const
{
    return pimpl->m_potency * state.m_statics.m_targets;
}

EnhScatter_t::EnhScatter_t()
{
    pimpl->m_name = "Scatter";
    pimpl->m_char = "*";
    pimpl->m_type = SPELL_TYPE;
    pimpl->m_potency = 100;
    pimpl->m_white = 8;
    pimpl->m_black = 8;
}

void EnhScatter_t::Execute(State_t & state) const
{
    if (rand() % 100 < 25)
        state.m_status.m_scatter = 10 * 1000;
    else
        state.m_status.m_scatter = 0;

    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool EnhScatter_t::IsUseable(const State_t & state) const
{
    return (state.m_status.m_scatter > 0);
}

int EnhScatter_t::CalculatePotency(const State_t & state) const
{
    return pimpl->m_potency * state.m_statics.m_targets;
}

Moulinet_t::Moulinet_t()
{
    pimpl->m_name = "NO MANA Moulinet";
    pimpl->m_char = "M";
    pimpl->m_type = WEAPONSKILL_TYPE;
    pimpl->m_is_magical_damage = false;
    pimpl->m_cast = 0;
    pimpl->m_recast = 2500;
    pimpl->m_potency = 60;
}

void Moulinet_t::Execute(State_t & state) const
{
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool Moulinet_t::IsUseable(const State_t & state) const
{
    return ((state.m_caststate.m_out_of_range == 0) &&
        !(state.m_mana.white() >= 30 && state.m_mana.black() >= 30));
}

int Moulinet_t::CalculatePotency(const State_t & state) const
{
    return pimpl->m_potency * state.m_statics.m_targets;
}

EnhMoulinet_t::EnhMoulinet_t()
{
    pimpl->m_name = "Moulinet";
    pimpl->m_char = "M";
    pimpl->m_type = WEAPONSKILL_TYPE;
    pimpl->m_is_magical_damage = true;
    pimpl->m_cast = 0;
    pimpl->m_recast = 1500;
    pimpl->m_potency = 200;
    pimpl->m_white = -30;
    pimpl->m_black = -30;
}

void EnhMoulinet_t::Execute(State_t & state) const
{
    state.m_melee_combo = State_t::MELEE_RIPOSTE;
}

bool EnhMoulinet_t::IsUseable(const State_t & state) const
{
    return ((state.m_caststate.m_out_of_range == 0) &&
        (state.m_mana.white() >= 30 && state.m_mana.black() >= 30));
}

int EnhMoulinet_t::CalculatePotency(const State_t & state) const
{
    return pimpl->m_potency * state.m_statics.m_targets;
}

Infusion_t::Infusion_t()
{
    pimpl->m_name = "Infusion of Intelligence";
    pimpl->m_char = "I";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_recast = (int)(4.5 * 60) * 1000;
    pimpl->m_animation_lock = 1350;
}

void Infusion_t::Execute(State_t & state) const
{
    state.m_status.m_infusion = 30 * 1000;
}

bool Infusion_t::IsUseable(const State_t & state) const
{
    return (state.m_recast.m_infusion == 0);
}

EndPlaceholder_t::EndPlaceholder_t()
{
    pimpl->m_name = "End";
    pimpl->m_char = "END";
    pimpl->m_type = FINISHER_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_recast = 0;
    pimpl->m_animation_lock = 0;
}

DiversionPlaceholder_t::DiversionPlaceholder_t()
{
    pimpl->m_name = "Diversion";
    pimpl->m_char = "_";
    pimpl->m_type = ABILITY_TYPE;
    pimpl->m_cast = 0;
    pimpl->m_recast = 120 * 1000;
}
