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

class ne_genome
{
    
public:
    
    ne_genome() {}
    
    ne_genome(const ne_genome& genome);
    
    ne_genome& operator = (const ne_genome& genome);
    
    ~ne_genome() {
        _destory();
    }
    
    inline ne_node** inputs() {
        return nodes.data();
    }
    
    inline ne_node** outputs() {
        return nodes.data() + input_size;
    }
    
    inline uint64 node_count() const {
        return nodes.size();
    }
    
    inline uint64 gene_count() const {
        return genes.size();
    }
    
    static inline bool compare(const ne_genome* a, const ne_genome* b) {
        return a->fitness > b->fitness;
    }
    
    void write(std::ofstream& out) const;
    void read(std::ifstream& in);
    
    void _destory();
    
    void reset(uint64 inputs, uint64 outputs);
    
    void initialize(ne_innovation_map* map, uint64* innovation);
    
    void insert(ne_node* node);
    
    void insert(ne_gene* gene);
    void pass_down(ne_gene* gene);
    
    void flush();
    
    bool done() const;
        
    void _compute(const ne_params& params);
            
    void mutate_weights(const ne_params& params);
    
    void mutate_add_node(ne_innovation_map* map, uint64* innovation, uint64* node_ids, const ne_params& params);
    
    void mutate_add_gene(ne_innovation_map* map, uint64* innovation, const ne_params& params);
        
    static ne_genome* crossover(const ne_genome* A, const ne_genome* B, const ne_params& params);
    static float64 distance(const ne_genome* A, const ne_genome* B, const ne_params& params);
    
    float64 fitness;
    bool eliminated;
    
private:
    
    ne_innovation_set set;
    ne_nodes_map nodes_map;
    
    uint64 input_size;
    uint64 output_size;
    
    uint64 activations;
    
    std::vector<ne_node*> nodes;
    std::vector<ne_gene*> genes;
};

inline void ne_mutate(ne_genome* genome, ne_innovation_map* map, uint64* innovation, uint64* node_ids, const ne_params& params) {
    if(random(0.0, 1.0) < params.new_node_prob) {
        genome->mutate_add_node(map, innovation, node_ids, params);
    }
    
    if(random(0.0, 1.0) < params.new_gene_prob) {
        genome->mutate_add_gene(map, innovation, params);
    }
    
    if(random(0.0, 1.0) < params.mutate_weights_prob) {
        genome->mutate_weights(params);
    }
}


#endif /* genome_h */
