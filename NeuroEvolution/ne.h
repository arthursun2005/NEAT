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

#define ne_func(x) (x / (1.0f + fabsf(x)))

struct ne_params {
    float weights_power;
    float species_thresh;
    float kill_ratio;
    float interspecies_mate_prob;
    float new_node_prob;
    float new_link_prob;
    float toggle_link_enable_prob;
    float mutate_weights_prob;
    float mate_prob;
    float weights_reset_prob;
    float weights_mutation_power;
    float mate_avg_prob;
    float disable_inheritance;
    
    size_t dead_species_age;
    size_t population;
    size_t input_size;
    size_t output_size;
    
    ne_params() {
        weights_power = 0.5f;
        species_thresh = 1.0f;
        kill_ratio = 0.5f;
        interspecies_mate_prob = 0.01f;
        new_node_prob = 0.03f;
        new_link_prob = 0.05f;
        toggle_link_enable_prob = 0.2f;
        mutate_weights_prob = 0.75f;
        mate_prob = 0.75f;
        weights_reset_prob = 0.1f;
        weights_mutation_power = 0.5f;
        mate_avg_prob = 0.5f;
        disable_inheritance = 0.75f;
        
        dead_species_age = 16;
        population = 256;
    }
};


inline float ne_random() {
    return rand32() / (float) (0xffffffff);
}

inline float ne_random(float a, float b) {
    return ne_random() * (b - a) + a;
}

struct ne_link {
    size_t i;
    size_t j;
    
    float weight;
    
    size_t innov;
    
    bool enabled;
};

struct ne_innov
{
    size_t i;
    size_t j;
    
    ne_innov(const ne_link& link) : i(link.i), j(link.j) {}
};

typedef std::unordered_map<ne_innov, size_t> ne_innov_map;
typedef std::unordered_set<ne_innov> ne_innov_set;

struct ne_node
{
    float value;
    float sum;
    
    size_t acts;
    bool computed;
};

inline size_t ne_hashi2(size_t i, size_t j) {
    return (i ^ j) + 71 * i;
}

template <>
struct std::hash<ne_innov>
{
    inline size_t operator () (const ne_innov& x) const {
        return ne_hashi2(x.i, x.j);
    }
};

template <>
struct std::equal_to<ne_innov>
{
    inline bool operator () (const ne_innov& a, const ne_innov& b) const {
        return a.i == b.i && a.j == b.j;
    }
};

#endif /* ne_h */
