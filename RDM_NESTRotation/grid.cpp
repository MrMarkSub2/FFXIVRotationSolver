#include "stdafx.h"
#include "grid.h"
#include "dps.h"
#include "movestring.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>

namespace {
    const int g_impactBitmask = 0x1;
    const int g_verstoneBitmask = 0x2;
    const int g_verfireBitmask = 0x4;
    const int g_maxBitmask = 0x8;
}

int boolsToBitmask(bool impact, bool verstone, bool verfire) {
    int rval = 0;
    if (impact) rval |= g_impactBitmask;
    if (verstone) rval |= g_verstoneBitmask;
    if (verfire) rval |= g_verfireBitmask;

    return rval;
}

void bitmaskToBools(int bitmask, bool& impact, bool& verstone, bool& verfire) {
    impact = bitmask & g_impactBitmask;
    verstone = bitmask & g_verstoneBitmask;
    verfire = bitmask & g_verfireBitmask;
}

template<class T>
void swap(Grid_t<T> & g1, Grid_t<T> & g2)
{
    std::swap(g1.pimpl, g2.pimpl);
}

template<class T>
int calcGridChanged(const Grid_t<T>& g1, const Grid_t<T>& g2)
{
    int rval = 0;

    for (int whiteMana = 0; whiteMana <= 100; ++whiteMana) {
        for (int blackMana = 0; blackMana <= 100; ++blackMana) {
            for (int bitmask = 0; bitmask < g_maxBitmask; ++bitmask) {
                if (!(g1.get(Mana_t(whiteMana, blackMana), bitmask) ==
                    g2.get(Mana_t(whiteMana, blackMana), bitmask))) {
                    ++rval;
                }
            }
        }
    }

    return rval;
}

template<class T>
struct Grid_t<T>::Impl {
    Impl();
    Impl(const T& defaultVal);
    std::vector<std::vector<std::vector<T> > > grid;
    bool statsCached;

    void printHelper(std::ostream& os);
};

template<class T>
Grid_t<T>::Impl::Impl()
    : grid(std::vector<std::vector<std::vector<T> > >(g_maxBitmask,
        std::vector<std::vector<T> >(101,
            std::vector<T>(101, T())))), statsCached(false)
{
    // 0 - maxbitmask impact, verstone, manification, etc
    // 0 - 100 white
    // 0 - 100 black
}

template<class T>
Grid_t<T>::Impl::Impl(const T& defaultVal)
    : grid(std::vector<std::vector<std::vector<T> > >(g_maxBitmask,
        std::vector<std::vector<T> >(101,
            std::vector<T>(101, defaultVal)))), statsCached(false)
{
    // 0 - maxbitmask impact, verstone, manification, etc
    // 0 - 100 white
    // 0 - 100 black
}

void printHelper(std::ostream& os, const DPS_t& dps) {
    os << dps;
}

void printHelper(std::ostream& os, std::shared_ptr<MoveString_t> ms) {
    os << *ms;
}

template<class T>
void Grid_t<T>::Impl::printHelper(std::ostream& os) {
    // TODO: uncomment this
    std::vector<std::vector<std::vector<T> > >::const_iterator bitmask_it = grid.begin();
    //for (std::vector<std::vector<std::vector<T> > >::const_iterator bitmask_it = grid.begin(); bitmask_it != grid.end(); ++bitmask_it) {
        for (std::vector<std::vector<T> >::const_iterator white_it = bitmask_it->begin(); white_it != bitmask_it->end(); ++white_it) {
            bool first(true);
            for (std::vector<T>::const_iterator black_it = white_it->begin(); black_it != white_it->end(); ++black_it) {
                if (first)
                    first = false;
                else
                    os << ',';
                ::printHelper(os, *black_it);
            }
            os << std::endl;
        }
    //}
}

template<class T> 
Grid_t<T>::Grid_t() : pimpl(new Impl) { }

template<class T>
Grid_t<T>::Grid_t(const T & defaultVal) : pimpl(new Impl(defaultVal)) { }

template<class T>
Grid_t<T>::Grid_t(const Grid_t<T>& g) : pimpl(new Impl(*g.pimpl)) { }

template<class T>
Grid_t<T>& Grid_t<T>::operator=(Grid_t<T> g) {
    swap(*this, g);

    return *this;
}

template<class T> Grid_t<T>::~Grid_t() { delete pimpl; }

template<class T>
const T& Grid_t<T>::get(const Mana_t& mana, bool impactProc, bool verstoneProc, bool verfireProc) const {
    return get(mana, boolsToBitmask(impactProc, verstoneProc, verfireProc));
}

template<class T>
const T & Grid_t<T>::get(const Mana_t & mana, int bitmask) const
{
    return pimpl->grid[bitmask][mana.white()][mana.black()];
}

template<class T>
void Grid_t<T>::set(const Mana_t & mana, bool impactProc, bool verstoneProc, bool verfireProc, const T & t)
{
    set(mana, boolsToBitmask(impactProc, verstoneProc, verfireProc), t);
}

template<class T>
void Grid_t<T>::set(const Mana_t & mana, int bitmask, const T & t)
{
    pimpl->grid[bitmask][mana.white()][mana.black()] = t;

}

template<class T>
void Grid_t<T>::print() const {
    pimpl->printHelper(std::cout);
}

template<class T>
void Grid_t<T>::printFile(const std::string& filename) const {
    std::ofstream os(filename);
    pimpl->printHelper(os);
}
