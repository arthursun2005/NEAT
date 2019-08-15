//
//  genome.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/15/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef genome_h
#define genome_h

#include "ne.h"

struct ne_species;

struct ne_genome
{
    ne_genome() {}
    
    ne_genome(const ne_genome& network);
    
    ne_genome& operator = (const ne_genome& network);
    
    ~ne_genome() {
        clear();
    }
    
    inline ne_node* inputs() {
        return nodes.data();
    }
    
    inline ne_node* outputs() {
        return nodes.data() + input_size;
    }
    
    inline size_t size() const {
        return nodes.size();
    }
    
    inline size_t complexity() const {
        return genes.size();
    }
    
    void write(std::ofstream& out);
    void read(std::ifstream& in);
    
    void clear();
    
    size_t create_node();
    
    void reset(size_t inputs, size_t outputs);
    
    void initialize(ne_innov_map* map, size_t* innov);
    
    void insert(ne_gene* gene);
    
    void flush();
    
    void compute(const ne_params& params);
        
    void mutate_weights(const ne_params& params);
    
    void mutate_topology_add_node(ne_innov_map* map, size_t* innov, const ne_params& params);
    
    void mutate_topology_add_link(ne_innov_map* map, size_t* innov, const ne_params& params);
    
    void mutate_toggle_link_enable(size_t times);
    
    unsigned int fitness;
    
    ne_species* sp;
    
    bool killed;
    
    ne_innov_set set;
    
    size_t input_size;
    size_t output_size;
    
    std::vector<ne_node> nodes;
    std::vector<ne_gene*> genes;
};

inline bool ne_genome_sort(const ne_genome* a, const ne_genome* b) {
    return a->fitness < b->fitness;
}

void ne_crossover(const ne_genome* A, const ne_genome* B, ne_genome* C, const ne_params& params);
float ne_distance(const ne_genome* A, const ne_genome* B, const ne_params& params);

inline void ne_mutate(ne_genome* genome, const ne_params& params, ne_innov_map* map, size_t* innov) {
    if(ne_random() < params.mutate_weights_prob) {
        genome->mutate_weights(params);
    }
    
    if(ne_random() < params.toggle_link_enable_prob) {
        genome->mutate_toggle_link_enable(1);
    }
    
    if(ne_random() < params.new_node_prob) {
        genome->mutate_topology_add_node(map, innov, params);
    }
    
    if(ne_random() < params.new_link_prob) {
        genome->mutate_topology_add_link(map, innov, params);
    }
}


#endif /* genome_h */