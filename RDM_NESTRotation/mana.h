#pragma once

#ifdef RDM_NESTROTATION_EXPORTS
#define RDM_NESTROTATION_API __declspec(dllexport)
#else
#define RDM_NESTROTATION_API __declspec(dllimport)
#endif

class RDM_NESTROTATION_API Mana_t {
public:
    Mana_t(int white, int black);
    Mana_t& add(int white, int black);
    int white() const { return m_white; }
    int black() const { return m_black; }

private:
    bool isUnbalancedWhite() const;
    bool isUnbalancedBlack() const;
    void cap();

    int m_white, m_black;
};
