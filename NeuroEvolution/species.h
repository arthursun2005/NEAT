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

struct ne_species
{
    std::vector<ne_genome*> genomes;
    
    float64 avg_fitness;
    
    uint64 alive_after;
    uint64 parents;
    uint64 offsprings;
};

inline bool ne_species_sort (const ne_species* A, const ne_species* B) {
    return A->avg_fitness > B->avg_fitness;
}

#endif /* species_h */
