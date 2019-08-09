//
//  Species.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/5/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef Species_h
#define Species_h

#include "Network.h"

namespace NE {

    struct Species
    {
        std::vector<Network*> networks;
        float avg_fitness;
        size_t offsprings;
    };
    
    inline bool species_sort (const Species& A, const Species& B) {
        return A.avg_fitness < B.avg_fitness;
    }
    
}

#endif /* Species_h */
