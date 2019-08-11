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

namespace NE {
    
    static float weights_power = 0.5f;
    static float species_thresh = 1.0f;
    static float survival_thresh = 0.2f;
    static float interspecies_mate_prob = 0.001f;
    static float new_node_prob = 0.03f;
    static float new_link_prob = 0.05f;
    static float toggle_link_enable_prob = 0.1f;
    static float toggle_node_enable_prob = 0.1f;
    static float mutate_weights_prob = 0.75f;
    static float mate_prob = 0.75f;
    static float weights_reset_prob = 0.1f;
    static float weights_mutate_prob = 0.8f;
    static float weights_mutation_power = 0.5f;
    static float mate_avg_prob = 0.5f;
    static float disable_inheritance = 0.75f;
    
    static size_t dead_species_age = 16;
    static size_t timeout = 16;

    inline uint32_t rand32() {
        return arc4random();
    }
    
    inline uint64_t rand64() {
        return rand32() | (uint64_t(rand32()) << 32);
    }
    
    inline void* malloc(size_t size) {
        return ::operator new(size);
    }
    
    inline void mfree(void* ptr) {
        ::operator delete(ptr);
    }
    
    inline float gaussian_random() {
        thread_local std::default_random_engine g;
        thread_local std::normal_distribution<float> d(0.0f, 1.0f);
        return d(g);
    }
    
    inline float random() {
        return rand32() / (float) (0xffffffff);
    }
    
    inline float randposneg() {
        return (rand32() & 1) ? -1.0f : 1.0f;
    }
    
    struct Function
    {
        inline float operator () (float x) const {
            return x / (1.0f + fabsf(x));
        }
    };
    
}

#endif /* common_h */
