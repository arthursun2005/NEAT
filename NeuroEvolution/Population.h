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
        for(ne_genome* genome : genomes) {
            delete genome;
        }
        
        for(ne_species* sp : species) {
            delete sp;
        }
    }
    
    ne_population() {}
    
    ne_population(const ne_population& population) {
        *this = population;
    }
    
    ne_population& operator = (const ne_population& population) {
        params = population.params;
        
        _kill();
        
        genomes.resize(params.population);
        
        for(uint64 i = 0; i < params.population; ++i) {
            genomes[i] = new ne_genome(*population.genomes[i]);
        }
        
        innovation = population.innovation;
        
        return *this;
    }
    
    void reset(const ne_params& _params, uint64 input_size, uint64 output_size) {
        params = _params;
        
        _kill();
        
        genomes.resize(params.population);
        
        for(ne_genome*& genome : genomes) {
            genome = new ne_genome();
        }
        
        innovation = 0;
        map.clear();
        
        for(ne_genome* genome : genomes) {
            genome->reset(input_size, output_size);
            genome->initialize(&map, &innovation);
        }
        
        node_ids = input_size + output_size + 1;
    }
    
    inline ne_genome* operator [] (uint64 i) {
        return genomes[i];
    }
    
    inline void compute(ne_genome* g) const {
        g->_compute(params);
    }
    
    ne_genome* select();
    
    void reproduce();
    
    uint64 innovation;
    
    std::vector<ne_genome*> genomes;
    std::vector<ne_species*> species;
        
    ne_params params;
    
    uint64 alive_after;
    uint64 parents;
    
    uint64 node_ids;
    
private:
    
    ne_innovation_map map;
    
    ne_genome* _breed(ne_species* sp);
    
    void _kill();
        
    void _speciate();
    
};

#endif /* ne_population_h */
