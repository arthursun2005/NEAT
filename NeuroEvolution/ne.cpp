//
//  ne.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/17/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "ne.h"

const std::string ne_params::names[] = {
    "align_power",
    "weights_power",
    "trait_power",
    "compat_mod",
    "compat_thresh",
    "kill_ratio",
    "interspecies_mate_prob",
    "new_node_prob",
    "new_gene_prob",
    "trait_mutate_prob",
    "toggle_gene_enable_prob",
    "mutate_weights_prob",
    "mate_prob",
    "weights_mutation_power",
    "mate_avg_prob",
    "disable_inheritance",
    
    "activations",
    "timeout",
    "population",
    "num_of_species"
};

const uint64 ne_params::n = sizeof(ne_params::names) / sizeof(*ne_params::names);

uint64 ne_params::find_index(const std::string& name) const {
    for(uint64 i = 0; i < n; ++i) {
        if(names[i] == name)
            return i;
    }
        
    return -1;
}

bool ne_params::load(std::ifstream& in) {
    if(!in.is_open()) {
        return false;
    }

    std::string name;
    
    while(true) {
        if(!(in >> name)) break;
        uint64 i = find_index(name);

        if(i == -1) {
            std::cout << "Unknown name: " << name << std::endl;
            in >> name;
            continue;
        }
        
        if(is_float(i)) {
            float64 x;
            in >> x;
            (&align_power)[i] = x;
        }else{
            uint64 x;
            in >> x;
            ((uint64*)(&align_power))[i] = x;
        }
    }

    return true;
}

float64 ne_function::operator () (float64 x) const {
    switch (type) {
        case ne_elliot1:
            return 0.5 * x / (1.0 + fabs(x)) + 0.5;
            
        case ne_elliot2:
            return x / (1.0 + fabs(x));
            
        case ne_step1:
            return x > 0.0 ? 1.0 : 0.0;
            
        case ne_step2:
            return x > 0.0 ? 1.0 : -1.0;
            
        case ne_abs:
            return fabs(x);
            
        case ne_linear:
            return x;
            
        case ne_gaussian:
            return exp(-(x * x) * 0.5);
            
        default:
            return 0.0;
    }
}

uint64 get_innovation(ne_innovation_map* map, uint64* innovation, const ne_innovation& i) {
    ne_innovation_map::iterator it = map->find(i);
    
    if(it != map->end()) {
        return it->second;
    }else{
        uint64 new_innovation = (*innovation)++;
        map->insert({i, new_innovation});
        return new_innovation;
    }
}
