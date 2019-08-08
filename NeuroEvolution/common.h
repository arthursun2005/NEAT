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
    
    const float similar_rate = 2.0f;
    
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
        return rand32() <= 0x7fffffff ? -1.0 : 1.0;
    }
    
    struct Function
    {
        inline float operator () (float x) const {
            return tanhf(x);
        }
    };
}

#endif /* common_h */
