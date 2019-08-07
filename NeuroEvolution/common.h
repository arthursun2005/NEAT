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

namespace NE {
    
    typedef float float_t;
        
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
    
    inline float_t gaussian_random() {
        thread_local std::default_random_engine g;
        thread_local std::normal_distribution<float_t> d(0.0, 1.0);
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
        inline float_t operator () (float_t x) const {
            return x / (1.0f + fabs(x));
        }
    };
}

#endif /* common_h */
