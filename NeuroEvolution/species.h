//
//  species.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/5/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef species_h
#define species_h

#include "network.h"

struct ne_species
{
    std::vector<ne_network*> networks;
    
    float avg_fitness;
    float best_fitness;
    float max_fitness;
    
    float rank;
    
    size_t time_since_improvement;
    size_t offsprings;
    
    size_t midpoint;
    size_t alive_size;
};

inline bool ne_species_sort (const ne_species* A, const ne_species* B) {
    return A->rank > B->rank;
}

#endif /* species_h */
