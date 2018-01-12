#include "stdafx.h"
#include "mana.h"

Mana_t::Mana_t(int white, int black)
    : m_white(white), m_black(black) {
    cap();
}

Mana_t& Mana_t::add(int white, int black) {
    if (isUnbalancedWhite()) {
        m_white += white;
        m_black += black / 2;
    }
    else if (isUnbalancedBlack()) {
        m_white += white / 2;
        m_black += black;
    }
    else {
        m_white += white;
        m_black += black;
    }
    cap();

    return *this;
}

bool Mana_t::isUnbalancedWhite() const {
    return (m_white - m_black > 30);
}

bool Mana_t::isUnbalancedBlack() const {
    return (m_black - m_white > 30);
}

void Mana_t::cap() {
    if (m_white < 0)
        m_white = 0;
    else if (m_white > 100)
        m_white = 100;

    if (m_black < 0)
        m_black = 0;
    else if (m_black > 100)
        m_black = 100;
}
