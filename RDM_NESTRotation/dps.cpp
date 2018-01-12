#include "stdafx.h"
#include "dps.h"
#include "utils.h"

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

DPS_t& DPS_t::add(int pot, int ms) {
    m_pot += pot;
    m_ms += ms;

    return *this;
}

DPS_t& DPS_t::add(const DPS_t& rhs) {
    m_pot += rhs.m_pot;
    m_ms += rhs.m_ms;

    return *this;
}

DPS_t & DPS_t::multiply(double ratio)
{
    m_pot = (int)(m_pot * ratio);

    return *this;
}

DPS_t & DPS_t::weigh(double ratio)
{
    m_pot = (int)(m_pot * ratio);
    m_ms = (int)(m_ms * ratio);

    return *this;
}

double DPS_t::calc() const {
    if (m_ms == 0.0)
        return 0.0;
    return (double)m_pot / m_ms * 1000.0;
}

bool DPS_t::operator==(const DPS_t & rhs) const
{
    return IsEssentiallyEqual(calc(), rhs.calc());
}

std::ostream & operator<<(std::ostream & os, const DPS_t & dps)
{
    os << static_cast<int>(100 * dps.calc()) / 100.0;
    return os;
}

DPS_t& addDPS(DPS_t& dps, int pot, int ms) {
    dps.add(pot, ms);
    return dps;
}
