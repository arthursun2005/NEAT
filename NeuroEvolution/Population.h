//
//  population.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/31/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ne_population_h
#define ne_population_h

#include "network.h"
#include "species.h"

class ne_population
{
    
public:
    
    ~ne_population() {
        for(ne_network* network : networks) {
            delete network;
        }
        
        for(ne_species* sp : species) {
            delete sp;
        }
    }
    
    inline void initialize(const ne_params& _params) {
        params = _params;
        
        for(ne_network* network : networks) {
            delete network;
        }
        
        networks.resize(params.population);
        
        for(ne_network*& network : networks) {
            network = new ne_network();
        }
        
        map.clear();
        innovation = 0;
        
        for(ne_network* network : networks) {
            network->reset(params.input_size, params.output_size);
            network->initialize(&map, &innovation);
        }
        
        _speciate();
    }
    
    inline ne_network* operator [] (size_t i) {
        return networks[i];
    }
    
    ne_network* select();
    
    void reproduce();
    
    size_t innovation;
    
    std::vector<ne_network*> networks;
    std::vector<ne_species*> species;
    
    float best_fitness;
    
    ne_params params;
    
private:
    
    innov_map map;
    
    void _add(ne_network* n);
    
    void _remove(ne_network* n);
    
    inline void _speciate() {
        species.clear();
        
        for(ne_network* n : networks) {
            _add(n);
        }
    }
    
};

#endif /* ne_population_h */
