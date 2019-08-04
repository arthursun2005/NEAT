//
//  main.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include <iostream>
#include "Population.h"

NE::Population population;

#define time_limit 1000

#define time_step 0.01f

FILE* log_file;

int gens = 512;
int pop = 1024;

int trials = 12;

struct Obj
{
    static const int input_size;
    static const int output_size;
    
    NE::Network* net;
    
    float reward;
    
    virtual void run() = 0;
};

struct Pendulum : public Obj
{
    
    static const int input_size = 5;
    static const int output_size = 1;
    
    typedef NE::float_t float_t;
    
    float_t x;
    float_t vx;
    float_t a;
    float_t va;
    
    float_t g;
    float_t m_c;
    float_t m_p;
    float_t m;
    float_t l;
    float_t f;
    float_t b;
    
    float_t xt = 2.4f;
    
    Pendulum() {
        g = 9.82f;
        m_c = 0.5f;
        m_p = 0.5f;
        m = m_c + m_p;
        l = 0.6f;
        f = 20.0f;
        b = 0.1f;
    }
    
    void reset() {
        float stdev = 0.1f;
        
        x = NE::gaussian_randomf() * stdev;
        vx = NE::gaussian_randomf() * stdev;
        a = NE::gaussian_randomf() * stdev + M_PI;
        va = NE::gaussian_randomf() * stdev;
        
        x = (NE::randomf() - 0.5f) * 2.0f * 0.75f * xt;
    }
    
    void step(float dt) {
        NE::Node* inputs = net->inputs();
        NE::Node* outputs = net->outputs();
        
        float_t c = cosf(a);
        float_t s = sinf(a);
        
        inputs[0].value = vx;
        inputs[1].value = x;
        inputs[2].value = va;
        inputs[3].value = c;
        inputs[4].value = s;
        
        net->compute();
        
        float_t action = outputs[0].value;
        
        action = action < -1.0f ? -1.0f : (action > 1.0f ? 1.0f : action);
        
        action *= f;
        
        float_t va2 = va * va;
        float_t sc = s * c;
        float_t c2 = c * c;
        
        float_t vvx = (-2.0f * m_p * l * va2 * s + 3.0f * m_p * g * sc + 4.0f * action - 4.0f * b * vx) / (4.0f * m - 3.0f * m_p * c2);
        float_t vva = (-3.0f * m_p * l * va2 * sc + 6.0f * m * g * s + 6.0f * (action - b * vx) * c) / (4.0f * l * m - 3.0f * m_p * l * c2);
        
        vx = vx + vvx * dt;
        va = va + vva * dt;
        
        x = x + vx * dt;
        a = a + va * dt;
        
        float q = x / xt;
        
        q = q > 1.0f ? 1.0f : q;
        q = q < -1.0f ? -1.0f : q;
        
        reward += 0.5f * (cosf(a) + 1.0f) * (cosf(q * M_PI * 0.5f));
    }
    
    void run() {
        reset();
        
        for(int i = 0; i < time_limit; ++i) {
            step(time_step);
        }
    }
    
};

struct XOR : public Obj
{
    static const int input_size = 2;
    static const int output_size = 1;
    
    void run() {
        int a = NE::rand32() & 1;
        int b = NE::rand32() & 1;
        int c = a ^ b;
        
        NE::Node* inputs = net->inputs();
        NE::Node* outputs = net->outputs();
        
        inputs[0].value = a;
        inputs[1].value = b;
        
        net->compute();
        
        float d = (outputs[0].value > 0.0f) - c;
        reward += 1.0f - (d * d);
    }
};

typedef Pendulum obj_type;

std::vector<obj_type> objs(pop);

void initialize() {
    population.resize(pop);
    population.reset(obj_type::input_size, obj_type::output_size);
    
    for(int i = 0; i < pop; ++i) {
        objs[i].net = population[i];
    }
}

int main(int argc, const char * argv[]) {
    log_file = fopen("log.txt", "w");
    initialize();
    
    NE::Network* best;
    
    fprintf(log_file, "Population: %d \n", pop);
    
    fflush(log_file);
    
    for(int n = 0; n < gens; ++n) {
        for(int i = 0; i < pop; ++i) {
            objs[i].reward = 0.0f;
            
            for(int q = 0; q < trials; ++q) {
                objs[i].run();
            }
            
            objs[i].reward /= (float) trials;
            
            objs[i].net->fitness = objs[i].reward;
        }
        
        best = population.step();
        
        fprintf(log_file, "%15.4f %8zu %8zu \n", best->fitness, best->size(), best->complexity());
        
        fflush(log_file);
    }
    
    fclose(log_file);
    return 0;
}
