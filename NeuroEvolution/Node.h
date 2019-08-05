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

namespace NE {
    
    struct Link {
        size_t i;
        size_t j;
        
        float_t weight;
        
        size_t innovation;
    };
    
    struct Node
    {
        float_t value;
        float_t bias;
        size_t links;
        
        Function function;
        
        list<Link>::type* begin;
    };
    
}

#endif /* Node_h */
