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

int gens = 128;
int pop = 1024;

int print_every = 8;

int trials = 8;

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
    
    float x;
    float vx;
    float a;
    float va;
    
    float g;
    float m_c;
    float m_p;
    float m;
    float l;
    float f;
    float b;
    
    float xt = 2.4f;
    
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
        
        x = NE::gaussian_random() * stdev;
        vx = NE::gaussian_random() * stdev;
        a = NE::gaussian_random() * stdev + M_PI;
        va = NE::gaussian_random() * stdev;
        
        x = (NE::random() - 0.5f) * 2.0f * 0.75f * xt;
    }
    
    void step(float dt) {
        NE::Node* inputs = net->inputs();
        NE::Node* outputs = net->outputs();
        
        float c = cosf(a);
        float s = sinf(a);
        
        inputs[0].value = vx;
        inputs[1].value = x / xt;
        inputs[2].value = va;
        inputs[3].value = c;
        inputs[4].value = s;
        
        net->compute();
        
        float action = outputs[0].value;
                
        action = action < -1.0f ? -1.0f : (action > 1.0f ? 1.0f : action);
        
        action *= f;
        
        float va2 = va * va;
        float sc = s * c;
        float c2 = c * c;
        
        float vvx = (-2.0f * m_p * l * va2 * s + 3.0f * m_p * g * sc + 4.0f * action - 4.0f * b * vx) / (4.0f * m - 3.0f * m_p * c2);
        float vva = (-3.0f * m_p * l * va2 * sc + 6.0f * m * g * s + 6.0f * (action - b * vx) * c) / (4.0f * l * m - 3.0f * m_p * l * c2);
        
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
    
    fprintf(log_file, "Population: %d \n \n ", pop);
    
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
        
        fprintf(log_file, "\n\n\n");
        
        fprintf(log_file, "Generation %d: \n", n);
        
        if((n%print_every) == 0) {
        
            for(int i = 0; i < pop; ++i) {
                fprintf(log_file, "%5d # %5d: %5zu %8.2f %8.2f %5zu %5zu %5zu \n", n, i, population[i]->age, population[i]->fitness, population[i]->strength, population[i]->complexity(), population[i]->size(), population[i]->k);
            }
            
        }
        
        best = population.step();
        
        fprintf(log_file, "best fitness %15.4f size %8zu complexity %8zu \n", best->fitness, best->size(), best->complexity());
        
        fprintf(log_file, "%8zu\n", population.innovation);
        
        fflush(log_file);
    }
    
    fclose(log_file);
    return 0;
}
