//
//  population.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/31/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ne_population_h
#define ne_population_h

#include "species.h"

class ne_population
{
    
public:
    
    ~ne_population() {
        _kill();
    }
    
    ne_population() {}
    
    ne_population(const ne_population& population) {
        *this = population;
    }
    
    ne_population& operator = (const ne_population& population);
    
    void reset(const ne_params& _params, uint64 input_size, uint64 output_size);
    
    inline ne_genome* operator [] (uint64 i) {
        return genomes[i];
    }
    
    inline void compute(ne_genome* g) const {
        g->_compute(params);
    }
    
    ne_genome* select();
    
    void reproduce();
    
    std::vector<ne_genome*> genomes;
    std::vector<ne_species*> species;
    
    ne_params params;
    
    uint64 innovation;
    uint64 parents;
    uint64 node_ids;
    
private:
    
    ne_innovation_map map;
    
    ne_genome* _breed(ne_species* sp);
    
    void _kill();
        
    void _speciate();
    void _add(ne_genome* g);
    
};

#endif /* ne_population_h */
