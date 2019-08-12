//
//  ne.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ne_h
#define ne_h

#include "common.h"
#include <unordered_map>

struct ne_link {
    size_t i;
    size_t j;
    
    float weight;
    
    size_t innov;
    
    bool enabled;
};

struct ne_innov
{
    size_t i;
    size_t j;
    
    ne_innov(const ne_link* link) : i(link->i), j(link->j) {}
};

typedef std::unordered_map<ne_innov, size_t> innov_map;

struct ne_node
{
    float value;
    float sum;
    
    size_t acts;
    bool computed;
};

inline size_t ne_hashi2(size_t i, size_t j) {
    return (i ^ j) + 71 * i;
}

template <>
struct std::hash<ne_innov>
{
    inline size_t operator () (const ne_innov& x) const {
        return ne_hashi2(x.i, x.j);
    }
};

template <>
struct std::equal_to<ne_innov>
{
    inline bool operator () (const ne_innov& a, const ne_innov& b) const {
        return a.i == b.i && a.j == b.j;
    }
};

template <>
struct std::hash<ne_link*>
{
    inline size_t operator () (const ne_link* x) const {
        return ne_hashi2(x->i, x->j);
    }
};

template <>
struct std::equal_to<ne_link*>
{
    inline bool operator () (const ne_link* a, const ne_link* b) const {
        return a->i == b->i && a->j == b->j;
    }
};

#endif /* ne_h */
