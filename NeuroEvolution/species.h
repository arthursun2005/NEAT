//
//  species.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/5/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef species_h
#define species_h

#include "genome.h"
#include <limits>

struct ne_species
{
    ne_species() {
        max_fitness = -std::numeric_limits<double>::max();
        time_since_improvement = 0;
    }
    
    std::vector<ne_genome*> genomes;
    
    float64 avg_fitness;
    float64 max_fitness;
    
    uint64 time_since_improvement;
    uint64 parents;
    uint64 offsprings;
    
    static inline bool compare (const ne_species* A, const ne_species* B) {
        return A->avg_fitness > B->avg_fitness;
    }
};

#endif /* species_h */
