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
        
        for(size_t i = 0; i < params.population; ++i) {
            genomes[i] = new ne_genome(*population.genomes[i]);
        }
        
        innovation = population.innovation;
        
        return *this;
    }
    
    inline void reset(const ne_params& _params) {
        params = _params;
        
        _kill();
        
        genomes.resize(params.population);
        
        for(ne_genome*& genome : genomes) {
            genome = new ne_genome();
        }
        
        innovation = 0;
        
        for(ne_genome* genome : genomes) {
            genome->reset(params.input_size, params.output_size);
            genome->initialize(&map, &innovation);
        }
    }
    
    inline ne_genome* operator [] (size_t i) {
        return genomes[i];
    }
    
    inline void compute(ne_genome* g) const {
        g->_compute(params);
    }
    
    ne_genome* select();
    
    void reproduce();
    
    size_t innovation;
    
    std::vector<ne_genome*> genomes;
    std::vector<ne_species*> species;
        
    ne_params params;
    
    size_t alive_after;
    size_t parents;
    
private:
    
    ne_innov_map map;
    
    ne_genome* _breed(ne_species* sp);
    
    void _kill();
        
    void _speciate();
    
};

#endif /* ne_population_h */
