//
//  Node.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef Node_h
#define Node_h

#include "common.h"
#include <unordered_map>

namespace NE {
    
    struct Link {
        size_t i;
        size_t j;
        
        float weight;
        
        size_t innov;
        
        bool enabled;
    };
    
    struct Innov
    {
        size_t i;
        size_t j;
        
        Innov(const Link* link) : i(link->i), j(link->j) {}
    };
    
    typedef std::unordered_map<Innov, size_t> innov_map;
    
    struct Node
    {
        float value;
        float sum;
        
        float bias;
                
        Function function;
        
        size_t acts;
        bool computed;
    };
    
    inline size_t hashi2(size_t i, size_t j) {
        return (i ^ j) + 71 * i;
    }
    
}

template <>
struct std::hash<NE::Innov>
{
    inline size_t operator () (const NE::Innov& x) const {
        return NE::hashi2(x.i, x.j);
    }
};

template <>
struct std::equal_to<NE::Innov>
{
    inline bool operator () (const NE::Innov& a, const NE::Innov& b) const {
        return a.i == b.i && a.j == b.j;
    }
};

template <>
struct std::hash<NE::Link*>
{
    inline size_t operator () (const NE::Link* x) const {
        return NE::hashi2(x->i, x->j);
    }
};

template <>
struct std::equal_to<NE::Link*>
{
    inline bool operator () (const NE::Link* a, const NE::Link* b) const {
        return a->i == b->i && a->j == b->j;
    }
};

#endif /* Node_h */
