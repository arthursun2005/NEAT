//
//  ne.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/17/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "ne.h"

const std::string ne_params::names[] = {
    "weights_power",
    "compat_thresh",
    "survive_thresh",
    "interspecies_mate_prob",
    "new_node_prob",
    "new_gene_prob",
    "toggle_gene_enable_prob",
    "mutate_weights_prob",
    "mate_prob",
    "weights_mutation_power",
    "weights_mutation_rate",
    "mate_avg_prob",
    "disable_inheritance",
    
    "activations",
    "timeout",
    "population",
    "dropoff_age"
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
            (&weights_power)[i] = x;
        }else{
            uint64 x;
            in >> x;
            ((uint64*)(&weights_power))[i] = x;
        }
    }

    return true;
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
