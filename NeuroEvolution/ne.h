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
    float64 weights_power;
    float64 species_mod;
    float64 species_thresh;
    float64 kill_ratio;
    float64 interspecies_mate_prob;
    float64 new_node_prob;
    float64 new_gene_prob;
    float64 toggle_gene_enable_prob;
    float64 mutate_weights_prob;
    float64 mate_prob;
    float64 weights_reset_prob;
    float64 weights_mutation_power;
    float64 mate_avg_prob;
    float64 disable_inheritance;
    
    uint64 activations;
    uint64 timeout;
    uint64 population;
    uint64 num_of_species;

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

struct ne_link {
    const uint64 i;
    const uint64 j;
    
    float64 weight;
    
    ne_link(uint64 i, uint64 j) : i(i), j(j) {}
};

struct ne_gene
{
    ne_link* link;
    const uint64 innov;
    
    bool enabled;
    
    ne_gene(uint64 i, uint64 j, uint64 innov) : innov(innov) {
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
    const uint64 i;
    const uint64 j;
    
    ne_innov(const ne_link& link) : i(link.i), j(link.j) {}
    
    ne_innov(uint64 i, uint64 j) : i(i), j(j) {}
};

typedef std::unordered_map<ne_innov, uint64> ne_innov_map;
typedef std::unordered_set<ne_innov> ne_innov_set;

struct ne_node
{
    float64 value;
    float64 sum;
    bool computed;
    uint64 activations;
};

template <>
struct std::hash<ne_innov>
{
    inline uint64 operator () (const ne_innov& x) const {
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

uint64 get_innov(ne_innov_map* map, uint64* innov, const ne_innov& i);

#endif /* ne_h */
