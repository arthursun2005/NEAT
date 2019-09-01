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
    float64 mutate_activation_prob;
    float64 mate_avg_prob;
    
    union {
        uint64 timeout;
        uint64 begin_uint;
    };
    
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

enum ne_innovation_type
{
    ne_new_node,
    ne_new_gene
};

struct ne_innovation
{
    ne_innovation_type type;
    
    uint64 i;
    uint64 j;
    uint64 innovation;
    uint64 id;
    
    inline ne_innovation(ne_gene* gene, ne_innovation_type type) : i(gene->i->id), j(gene->j->id), type(type) {}
};

inline size_t ne_hash(uint64 i, uint64 j) {
    return (((i + j) * (i + j + 1)) >> 1) + j;
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
        return a.type == b.type && a.i == b.i && a.j == b.j;
    }
};

typedef std::unordered_set<ne_gene*, ne_gene_hash, ne_gene_equal> ne_gene_set;
typedef std::unordered_set<ne_innovation, ne_innovation_hash, ne_innovation_equal> ne_innovation_set;
typedef std::unordered_map<uint64, ne_node*> ne_nodes_map;

inline void ne_get_innovation(ne_innovation_set* set, uint64* innovation, uint64* nodes_id, ne_innovation* p) {
    ne_innovation_set::iterator it = set->find(*p);
    
    if(it != set->end()) {
        p->innovation = it->innovation;
        p->id = it->id;
    }else{
        p->innovation = *innovation;
        
        if(p->type == ne_new_node) {
            p->id = (*nodes_id)++;
            *innovation += 2;
        }else{
            *innovation += 1;
        }
        
        set->insert(*p);
    }
}

#endif /* ne_h */
