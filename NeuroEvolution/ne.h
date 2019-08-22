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

inline float64 ne_function (float64 x) {
    return x / (1.0 + fabs(x));
}

struct ne_params {
    float64 weights_power;
    float64 compat_thresh;
    float64 survive_thresh;
    float64 interspecies_mate_prob;
    float64 new_node_prob;
    float64 new_gene_prob;
    float64 toggle_gene_enable_prob;
    float64 mutate_weights_prob;
    float64 mate_prob;
    float64 weights_mutation_power;
    float64 weights_mutation_rate;
    float64 mate_avg_prob;
    float64 disable_inheritance;
    
    uint64 activations;
    uint64 timeout;
    uint64 population;
    uint64 dropoff_age;

    static const std::string names[];

    static const uint64 n;
    
    inline bool is_float(uint64 i) const {
        return i < ((uint64*)&activations - (uint64*)&weights_power);
    }
    
    uint64 find_index(const std::string& name) const;
        
    bool load(std::ifstream& in);
};

inline float64 ne_random() {
    return rand64() / (float64) (0xffffffffffffffff);
}

inline float64 ne_random(float64 a, float64 b) {
    return ne_random() * (b - a) + a;
}

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
    
    bool enabled;
    
    ne_gene() {}
    
    ne_gene(ne_node* i, ne_node* j) : i(i), j(j) {}
};

struct ne_innovation
{
    uint64 i;
    uint64 j;
    
    ne_innovation(const ne_gene& gene) : i(gene.i->id), j(gene.j->id) {}
    
    ne_innovation(uint64 i, uint64 j) : i(i), j(j) {}
};

typedef std::unordered_map<ne_innovation, uint64> ne_innovation_map;
typedef std::unordered_set<ne_innovation> ne_innovation_set;

typedef std::unordered_map<uint64, ne_node*> ne_nodes_map;

template <>
struct std::hash<ne_innovation>
{
    inline uint64 operator () (const ne_innovation& x) const {
        return (x.i ^ x.j) + 3331 * x.i;
    }
};

template <>
struct std::equal_to<ne_innovation>
{
    inline bool operator () (const ne_innovation& a, const ne_innovation& b) const {
        return a.i == b.i && a.j == b.j;
    }
};

uint64 get_innovation(ne_innovation_map* map, uint64* innovation, const ne_innovation& i);

#endif /* ne_h */
