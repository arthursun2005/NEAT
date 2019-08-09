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
                
        Function function;
        
        size_t acts;
        bool computed;
    };
    
}

template <>
struct std::hash<NE::Innov>
{
    inline size_t operator () (const NE::Innov& x) const {
        return (x.i ^ x.j) + 71 * x.i;
    }
};

template <>
struct std::equal_to<NE::Innov>
{
    inline bool operator () (const NE::Innov& a, const NE::Innov& b) const {
        return a.i == b.i && a.j == b.j;
    }
};

#endif /* Node_h */
