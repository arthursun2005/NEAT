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
#include <list>

namespace NE {
    
    struct Link {
        size_t i;
        size_t j;
        
        float_t weight;
        
        size_t innovation;
        
        std::list<Link*>::iterator innov_it;
    };
    
    struct LinkByInnov
    {
        inline bool operator () (Link* A, Link* B) const {
            return A->innovation < B->innovation;
        }
    };
    
    struct Node
    {
        float_t value;
        size_t links;
        
        Function function;
        
        std::list<Link*>::iterator begin;
    };
    
}

#endif /* Node_h */
