//
//  common.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef common_h
#define common_h

#include <cmath>
#include <functional>
#include <random>
#include <cfloat>

#define ne_func(x) (x / (1.0f + fabsf(x)))

struct ne_params {
    float weights_power;
    float species_thresh;
    float kill_thresh;
    float interspecies_mate_prob;
    float new_node_prob;
    float new_link_prob;
    float toggle_link_enable_prob;
    float mutate_weights_prob;
    float mate_prob;
    float weights_reset_prob;
    float weights_mutate_prob;
    float weights_mutation_power;
    float mate_avg_prob;
    float disable_inheritance;
    
    size_t dead_species_age;
    size_t timeout;
    size_t population;
    size_t input_size;
    size_t output_size;
    
    ne_params() {
        weights_power = 1.0f;
        species_thresh = 0.5f;
        kill_thresh = 0.75f;
        interspecies_mate_prob = 0.001f;
        new_node_prob = 0.03f;
        new_link_prob = 0.05f;
        toggle_link_enable_prob = 0.1f;
        mutate_weights_prob = 0.75f;
        mate_prob = 0.75f;
        weights_reset_prob = 0.1f;
        weights_mutate_prob = 0.8f;
        weights_mutation_power = 0.5f;
        mate_avg_prob = 0.5f;
        disable_inheritance = 0.75f;
        
        dead_species_age = 16;
        timeout = 8;
        population = 256;
    }
};
    
inline uint32_t rand32() {
    return arc4random();
}

inline uint64_t rand64() {
    return rand32() | (uint64_t(rand32()) << 32);
}
    
inline float gaussian_random() {
    thread_local std::default_random_engine g;
    thread_local std::normal_distribution<float> d(0.0f, 1.0f);
    return d(g);
}

inline float ne_random() {
    return rand32() / (float) (0xffffffff);
}

inline float ne_random(float a, float b) {
    return ne_random() * (b - a) + a;
}

#endif /* common_h */
