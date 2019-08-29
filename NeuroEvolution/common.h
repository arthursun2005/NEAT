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
#include <cfloat>
#include <iostream>

typedef float float32;
typedef double float64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

inline uint32 rand32() {
    return arc4random();
}

inline uint64 rand64() {
    return rand32() | ((uint64)(rand32()) << 32);
}

inline float64 random(float64 a, float64 b) {
    return (rand64() / (float64) (0xffffffffffffffff)) * (b - a) + a;
}

inline uint64 random(uint64 a, uint64 b) {
    return random(0.0, 1.0) * (b - a) + a;
}

inline float64 gaussian_random() {
    static bool computed = false;
    static float64 value;
    
    if(computed) {
        computed = false;
        return value;
    }else{
        float64 a, b, r, q;
        
        do {
            a = random(-1.0, 1.0);
            b = random(-1.0, 1.0);
            r = a * a + b * b;
        }  while (r >= 1.0 || r == 0.0);
        
        q = sqrt(-2.0 * log(r) / r);
        
        computed = true;
        value = a * q;
        return b * q;
    }
}

#endif /* common_h */
