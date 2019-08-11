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
    
    static float weights_power = 1.0f;
    static float similar_rate = 2.0f;
    static float survival_thresh = 0.5f;
    static float interspecies_mate_rate = 0.01f;
    static float mutate_topology_rate = 0.2f;
    static float mutate_weights_rate = 0.75f;
    static float mate_rate = 0.75f;
    static float weights_reset_rate = 0.1f;
    static float weights_mutate_rate = 0.75f;
    static float weights_mutation_power = 2.0f;
    static float mate_avg_rate = 0.5f;
    static size_t dead_species_rate = 16;
    
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
    
    inline double hebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate) {
        
        bool neg=false;
        double delta;
        
        //double weight_mag;
        
        double topweight;
        
        if (maxweight<5.0) maxweight=5.0;
        
        if (weight>maxweight) weight=maxweight;
        
        if (weight<-maxweight) weight=-maxweight;
        
        if (weight<0) {
            neg=true;
            weight=-weight;
        }
        
        
        //if (weight<0) {
        //  weight_mag=-weight;
        //}
        //else weight_mag=weight;
        
        
        topweight=weight+2.0;
        if (topweight>maxweight) topweight=maxweight;
        
        if (!(neg)) {
            //if (true) {
            delta=
            hebb_rate*(maxweight-weight)*active_in*active_out+
            pre_rate*(topweight)*active_in*(active_out-1.0);
            //post_rate*(weight+1.0)*(active_in-1.0)*active_out;
            
            //delta=delta-hebb_rate/2; //decay
            
            //delta=delta+randposneg()*randfloat()*0.01; //noise
            
            //cout<<"delta: "<<delta<<endl;
            
            //if (weight+delta>0)
            //  return weight+delta;
            //else return 0.01;
            
            return weight+delta;
            
        }
        else {
            //In the inhibatory case, we strengthen the synapse when output is low and
            //input is high
            delta=
            pre_rate*(maxweight-weight)*active_in*(1.0-active_out)+ //"unhebb"
            //hebb_rate*(maxweight-weight)*(1.0-active_in)*(active_out)+
            -hebb_rate*(topweight+2.0)*active_in*active_out+ //anti-hebbian
            //hebb_rate*(maxweight-weight)*active_in*active_out+
            //pre_rate*weight*active_in*(active_out-1.0)+
            //post_rate*weight*(active_in-1.0)*active_out;
            0;
            
            //delta=delta-hebb_rate; //decay
            
            //delta=delta+randposneg()*randfloat()*0.01; //noise
            
            //if (-(weight+delta)<0)
            //  return -(weight+delta);
            //  else return -0.01;
            
            return -(weight+delta);
            
        }
        
    }
}

#endif /* common_h */
