#pragma once

#include <ostream>

#ifdef RDM_NESTROTATION_EXPORTS
#define RDM_NESTROTATION_API __declspec(dllexport)
#else
#define RDM_NESTROTATION_API __declspec(dllimport)
#endif

class RDM_NESTROTATION_API DPS_t {
public:
    DPS_t()
        : m_pot(0), m_ms(0)
    {}
    DPS_t(int pot, int ms)
        : m_pot(pot), m_ms(ms)
    {}

    DPS_t& add(int pot, int ms);
    DPS_t& add(const DPS_t& rhs);

    // multiplies potency by ratio
    DPS_t& multiply(double ratio);
    // multiplies both potency and duration by ratio
    DPS_t& weigh(double ratio);

    double calc() const;
    bool operator==(const DPS_t& rhs) const;

    int getPot() const { return m_pot; }
    double getSec() const { return (double)m_ms/1000.0; }
    int getMS() const { return m_ms; }

    friend std::ostream& operator<<(std::ostream& os, const DPS_t& dps);

private:
    int m_pot;
    int m_ms;
};

RDM_NESTROTATION_API DPS_t& addDPS(DPS_t& dps, int pot, int ms);
