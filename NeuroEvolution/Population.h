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
        for(ne_species* sp : species) {
            delete sp;
        }
        
        for(ne_genome* genome : genomes) {
            delete genome;
        }
    }
    
    inline void reset(const ne_params& _params) {
        params = _params;
        
        for(ne_genome* genome : genomes) {
            delete genome;
        }
        
        genomes.resize(params.population);
        
        for(ne_genome*& genome : genomes) {
            genome = new ne_genome();
        }
        
        map.clear();
        innovation = 0;
        
        for(ne_genome* genome : genomes) {
            genome->reset(params.input_size, params.output_size);
            genome->initialize(&map, &innovation);
        }
        
        _speciate();
    }
    
    inline ne_genome* operator [] (size_t i) {
        return genomes[i];
    }
    
    inline void compute(ne_genome* g) const {
        g->compute(params);
    }
    
    ne_genome* select();
    
    void reproduce();
    
    size_t innovation;
    
    std::vector<ne_genome*> genomes;
    std::vector<ne_species*> species;
        
    ne_params params;
    
private:
    
    ne_innov_map map;
    
    ne_genome* _breed(ne_species* sp);
    
    void _kill();
    
    void _add(ne_genome* g);
    
    void _remove(ne_genome* g);
    
    inline void _speciate() {
        for(ne_species* sp : species) {
            delete sp;
        }
        
        species.clear();
        
        for(ne_genome* n : genomes) {
            _add(n);
        }
    }
    
};

#endif /* ne_population_h */
