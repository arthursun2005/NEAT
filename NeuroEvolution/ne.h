//
//  ne.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ne_h
#define ne_h

#include "common.h"
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <vector>

#define ne_func(x) (x / (1.0 + fabs(x)))

struct ne_params {
    double weights_power;
    double species_thresh;
    double kill_ratio;
    double interspecies_mate_prob;
    double new_node_prob;
    double new_gene_prob;
    double toggle_gene_enable_prob;
    double mutate_weights_prob;
    double mate_prob;
    double weights_reset_prob;
    double weights_mutation_power;
    double mate_avg_prob;
    double disable_inheritance;
    double species_mod;
    
    size_t activations;
    size_t timeout;
    size_t population;
    size_t input_size;
    size_t output_size;
    size_t num_of_species;
    
    ne_params() {
        num_of_species = 16;
        species_mod = 0.2;
        weights_power = 0.5;
        species_thresh = 1.0;
        kill_ratio = 0.5;
        interspecies_mate_prob = 0.01;
        new_node_prob = 0.03;
        new_gene_prob = 0.05;
        toggle_gene_enable_prob = 0.1;
        mutate_weights_prob = 0.75;
        mate_prob = 0.75;
        weights_reset_prob = 0.1;
        weights_mutation_power = 2.0;
        mate_avg_prob = 0.5;
        disable_inheritance = 0.75;
        
        activations = 16;
        timeout = 16;
        population = 256;
    }
};


inline double ne_random() {
    return rand64() / (double) (0xffffffffffffffff);
}

inline double ne_random(double a, double b) {
    return ne_random() * (b - a) + a;
}

struct ne_link {
    const size_t i;
    const size_t j;
    
    double weight;
    
    ne_link(size_t i, size_t j) : i(i), j(j) {}
};

struct ne_gene
{
    ne_link* link;
    const size_t innov;
    
    bool enabled;
    
    ne_gene(size_t i, size_t j, size_t innov) : innov(innov) {
        link = new ne_link(i, j);
    }
    
    ne_gene(const ne_gene& gene) : innov(gene.innov), enabled(gene.enabled) {
        link = new ne_link(*gene.link);
    }
    
    ~ne_gene() {
        delete link;
    }
};

struct ne_innov
{
    const size_t i;
    const size_t j;
    
    ne_innov(const ne_link& link) : i(link.i), j(link.j) {}
    
    ne_innov(size_t i, size_t j) : i(i), j(j) {}
};

typedef std::unordered_map<ne_innov, size_t> ne_innov_map;
typedef std::unordered_set<ne_innov> ne_innov_set;

struct ne_node
{
    double value;
    double sum;
    bool computed;
    size_t activations;
};

template <>
struct std::hash<ne_innov>
{
    inline size_t operator () (const ne_innov& x) const {
        return (x.i ^ x.j) + 331 * x.i;
    }
};

template <>
struct std::equal_to<ne_innov>
{
    inline bool operator () (const ne_innov& a, const ne_innov& b) const {
        return a.i == b.i && a.j == b.j;
    }
};

inline size_t get_innov(ne_innov_map* map, size_t* innov, const ne_innov& i) {
    auto it = map->find(i);
    
    if(it != map->end()) {
        return it->second;
    }else{
        size_t new_innov = *innov;
        ++(*innov);
        map->insert({i, new_innov});
        return new_innov;
    }
}

#endif /* ne_h */
