#pragma once

#include "dps.h"
#include "mana.h"
#include "movestring.h"
#include <string>

#ifdef RDM_NESTROTATION_EXPORTS
#define RDM_NESTROTATION_API __declspec(dllexport)
#define RDM_NESTROTATION_TEMPLATE
#else
#define RDM_NESTROTATION_API __declspec(dllimport)
#define RDM_NESTROTATION_TEMPLATE extern
#endif

template <class T>
class RDM_NESTROTATION_API Grid_t;

template <class T>
void swap(Grid_t<T>& g1, Grid_t<T>& g2);

template <class T>
class RDM_NESTROTATION_API Grid_t {
public:
    Grid_t();
    Grid_t(const T& defaultVal);
    Grid_t(const Grid_t& g);
    Grid_t& operator=(Grid_t g);
    ~Grid_t();

    friend void swap<T>(Grid_t& g1, Grid_t& g2);

    const T& get(const Mana_t& mana, bool impactProc, bool verstoneProc, bool verfireProc) const;
    const T& get(const Mana_t& mana, int bitmask) const;
    void set(const Mana_t& mana, bool impactProc, bool verstoneProc, bool verfireProc, const T& t);
    void set(const Mana_t& mana, int bitmask, const T& t);

    void print() const;
    void printFile(const std::string& filename) const;

private:
    struct Impl;
    Impl* pimpl;
};

template <class T>
int RDM_NESTROTATION_API calcGridChanged(const Grid_t<T>& g1, const Grid_t<T>& g2);

#pragma warning( push )
#pragma warning( disable : 4661)
RDM_NESTROTATION_TEMPLATE template class RDM_NESTROTATION_API Grid_t<DPS_t>;
RDM_NESTROTATION_TEMPLATE template class RDM_NESTROTATION_API Grid_t<std::shared_ptr<MoveString_t> >;
RDM_NESTROTATION_TEMPLATE template int RDM_NESTROTATION_API calcGridChanged(const Grid_t<DPS_t>&, const Grid_t<DPS_t>&);
RDM_NESTROTATION_TEMPLATE template int RDM_NESTROTATION_API calcGridChanged(const Grid_t<std::shared_ptr<MoveString_t> >&, const Grid_t<std::shared_ptr<MoveString_t> >&);
#pragma warning( pop )

int RDM_NESTROTATION_API boolsToBitmask(bool impact, bool verstone, bool verfire);
void RDM_NESTROTATION_API bitmaskToBools(int bitmask, bool & impact, bool & verstone, bool & verfire);
