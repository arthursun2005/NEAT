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

#define ne_function(x) (1.0 + x / (1.0 + fabs(x)))

struct ne_params {
    union {
        float64 weights_power;
        uint64 begin_float;
    };
    
    float64 compat_thresh;
    float64 survive_thresh;
    float64 interspecies_mate_prob;
    float64 new_node_prob;
    float64 new_gene_prob;
    float64 mutate_weights_prob;
    float64 mutate_only_prob;
    float64 mate_only_prob;
    float64 weights_mutation_power;
    float64 weights_mutation_rate;
    float64 disable_inheritance;
    
    union {
        uint64 activations;
        uint64 begin_uint;
    };
    
    uint64 timeout;
    uint64 population;
    uint64 dropoff_age;
    
    static const std::string names[];
    
    static const uint64 n;
    
    inline bool is_float(uint64 i) const {
        return i < (&begin_uint - &begin_float);
    }
    
    uint64 find_index(const std::string& name) const;
    
    bool load(std::ifstream& in);
};

struct ne_node
{
    float64 value;
    float64 sum;
    
    bool computed;
    
    uint64 activations;
    uint64 id;
};

struct ne_gene
{
    ne_node* i;
    ne_node* j;
    
    uint64 innovation;
    float64 weight;
    
    ne_gene() {}
    ne_gene(ne_node* i, ne_node* j) : i(i), j(j) {}
};

struct ne_innovation
{
    uint64 i;
    uint64 j;
};

inline size_t ne_hash(uint64 i, uint64 j) {
    return ((i * (i + 1)) >> 1) + j;
}

struct ne_gene_hash
{
    inline uint64 operator () (const ne_gene* x) const {
        return ne_hash(x->i->id, x->j->id);
    }
};

struct ne_gene_equal
{
    inline bool operator () (const ne_gene* a, const ne_gene* b) const {
        return a->i->id == b->i->id && a->j->id == b->j->id;
    }
};

struct ne_innovation_hash
{
    inline uint64 operator () (const ne_innovation& x) const {
        return ne_hash(x.i, x.j);
    }
};

struct ne_innovation_equal
{
    inline bool operator () (const ne_innovation& a, const ne_innovation& b) const {
        return a.i == b.i && a.j == b.j;
    }
};

typedef std::unordered_set<ne_gene*, ne_gene_hash, ne_gene_equal> ne_innovation_set;
typedef std::unordered_map<ne_innovation, uint64, ne_innovation_hash, ne_innovation_equal> ne_innovation_map;
typedef std::unordered_map<uint64, ne_node*> ne_nodes_map;

inline void set_innovation(ne_innovation_map* map, uint64* innovation, ne_gene* gene) {
    ne_innovation innov;
    
    innov.i = gene->i->id;
    innov.j = gene->j->id;
    
    ne_innovation_map::iterator it = map->find(innov);
    
    if(it != map->end()) {
        gene->innovation = it->second;
    }else{
        gene->innovation = (*innovation)++;
        map->insert({innov, gene->innovation});
    }
}

#endif /* ne_h */
