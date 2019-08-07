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
#include <unordered_set>

namespace NE {
    
    struct Link {
        size_t i;
        size_t j;
        
        float_t weight;
        
        size_t innov;
        
        bool enabled;
    };
    
    struct LinkByInnov
    {
        inline bool operator () (Link* A, Link* B) const {
            return A->innov < B->innov;
        }
    };

    typedef std::unordered_set<Link*> innov_set;
    
    struct Node
    {
        float_t value;
        float_t sum;
        
        Function function;
                
        size_t acts;
        bool computed;
    };
    
}

template <>
struct std::hash<NE::Link*>
{
    inline size_t operator () (const NE::Link* x) const {
        return (x->i ^ x->j) + (x->i << 17) + 71 * x->j;
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
