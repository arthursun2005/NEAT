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

inline uint32_t rand32() {
    return arc4random();
}

inline uint64_t rand64() {
    return rand32() | ((uint64_t)(rand32()) << 32);
}
    
inline double gaussian_random() {
    thread_local std::default_random_engine g;
    thread_local std::normal_distribution<double> d(0.0, 1.0);
    return d(g);
}

#endif /* common_h */
