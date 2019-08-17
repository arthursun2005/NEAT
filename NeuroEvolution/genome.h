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
    
    ne_genome(const ne_genome& genome);
    
    ne_genome& operator = (const ne_genome& genome);
    
    ~ne_genome() {
        _destory_genes();
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
    
    void _destory_genes();
    
    size_t create_node();
    
    void reset(size_t inputs, size_t outputs);
    
    void initialize(ne_innov_map* map, size_t* innov);
    
    void insert(ne_gene* gene);
    
    void flush();
    
    bool done() const;
        
    void _compute(const ne_params& params);
        
    void mutate_weights(const ne_params& params);
    
    void mutate_topology_add_node(ne_innov_map* map, size_t* innov, const ne_params& params);
    
    void mutate_topology_add_gene(ne_innov_map* map, size_t* innov, const ne_params& params);
    
    void mutate_toggle_gene_enable(size_t times);
    
    double fitness;
    double adjusted_fitness;
        
    ne_innov_set set;
    
    size_t input_size;
    size_t output_size;
    
    size_t activations;
    
    std::vector<ne_node> nodes;
    std::vector<ne_gene*> genes;
};

inline bool ne_genome_sort(const ne_genome* a, const ne_genome* b) {
    return a->fitness < b->fitness;
}

inline bool ne_genome_adjusted_sort(const ne_genome* a, const ne_genome* b) {
    return a->adjusted_fitness < b->adjusted_fitness;
}

void ne_crossover(const ne_genome* A, const ne_genome* B, ne_genome* C, const ne_params& params);
double ne_distance(const ne_genome* A, const ne_genome* B, const ne_params& params);

inline bool ne_mutate(ne_genome* genome, const ne_params& params, ne_innov_map* map, size_t* innov) {
    if(ne_random() < params.new_node_prob) {
        genome->mutate_topology_add_node(map, innov, params);
        return true;
    }
    
    if(ne_random() < params.new_gene_prob) {
        genome->mutate_topology_add_gene(map, innov, params);
        return true;
    }
    
    if(ne_random() < params.mutate_weights_prob) {
        genome->mutate_weights(params);
        return true;
    }
    
    if(ne_random() < params.toggle_gene_enable_prob) {
        genome->mutate_toggle_gene_enable(1);
        return true;
    }
    
    return false;
}


#endif /* genome_h */
