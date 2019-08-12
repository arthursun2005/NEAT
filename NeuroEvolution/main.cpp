//
//  main.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include <iostream>
#include "Population.h"

ne_population population;

#define time_limit 1000

#define time_step 0.01f

FILE* log_file;
FILE* pos_file;

int gens = 32;
int pop = 256;

int trials = 8;

struct Obj
{
    static const int input_size;
    static const int output_size;
    
    float reward;
    
    virtual void run(ne_network* net) = 0;
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
        g = 9.8f;
        m_c = 0.5f;
        m_p = 0.5f;
        m = m_c + m_p;
        l = 0.6f;
        f = 20.0f;
        b = 0.1f;
    }
    
    void reset() {
        float stdev = 0.1f;
        
        x = gaussian_random() * stdev;
        vx = gaussian_random() * stdev;
        a = gaussian_random() * stdev + M_PI;
        va = gaussian_random() * stdev;
    }
    
    void step(float dt, ne_network* net) {
        ne_node* inputs = net->inputs();
        ne_node* outputs = net->outputs();
        
        float c = cosf(a);
        float s = sinf(a);
        
        inputs[0].value = vx;
        inputs[1].value = x;
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
        
        if(q > -1.0f && q < 1.0f) {
            //reward += 0.5f * (cosf(a) + 1.0f);
        }
    }
    
    void run(ne_network* net) {
        for(int q = 0; q < trials; ++q) {
            reset();

            for(int i = 0; i < time_limit; ++i) {
                step(time_step, net);
            }
        }
        
        reward /= (float) trials;
    }
    
};

struct XOR : public Obj
{
    static const int input_size = 2;
    static const int output_size = 1;
    
    void run(ne_network* net) {
        for(int a = 0; a < 2; ++a) {
            for(int b = 0; b < 2; ++b) {
                int c = a ^ b;
                
                ne_node* inputs = net->inputs();
                ne_node* outputs = net->outputs();
                
                inputs[0].value = a;
                inputs[1].value = b;
                
                net->compute();
                
                float d = (outputs[0].value >= 1.0f) - c;
                reward += 1.0f - d * d;
            }
        }
    }
};

typedef Pendulum obj_type;

std::vector<obj_type> objs(pop);

void initialize() {
    ne_params params;
    params.input_size = Pendulum::input_size;
    params.output_size = Pendulum::output_size;
    params.population = pop;
    population.initialize(params);
}

int main(int argc, const char * argv[]) {
    log_file = fopen("log.txt", "w");
    pos_file = fopen("pos.txt", "w");
    
    initialize();
    
    ne_network* best = nullptr;
    
    fprintf(log_file, "Population: %d \n \n ", pop);
    
    fflush(log_file);

    for(int n = 0; n < gens; ++n) {
        for(int i = 0; i < pop; ++i) {
            objs[i].reward = 0.0f;
            
            objs[i].run(population[i]);
            
            population[i]->fitness = objs[i].reward;
        }
        
        fprintf(log_file, "\n\n\n");
        
        fprintf(log_file, "Generation %d: \n", n);
        
        best = population.select();
        
        for(int i = 0; i < pop; ++i) {
            fprintf(log_file, "%5d # %5d: age %5zu %6.2f complexity %5zu size %5zu \n", n, i, population[i]->age, population[i]->fitness, population[i]->complexity(), population[i]->size());
        }
        
        fprintf(log_file, "\n\n");
        
        for(ne_species* sp : population.species) {
            fprintf(log_file, "Species %6.2f offsprings %5zu size %5zu %5zu \n", sp->avg_fitness, sp->offsprings, sp->networks.size(), sp->time_since_improvement);
        }
        
        fprintf(log_file, "\n\n");
        
        fprintf(log_file, "best fitness %6.2f complexity %5zu size %5zu \n", best->fitness, best->complexity(), best->size());
        
        fprintf(log_file, "%5zu\n", population.innovation);
                
        fflush(log_file);
        
        population.reproduce();
    }
    
    Pendulum p;
    p.reward = 0.0f;
    
    p.reset();
    
    for(int i = 0; i < time_limit; ++i) {
        p.step(time_step, best);
        
        fprintf(pos_file, "%f, %f, \n", p.x, p.a);
    }
    
    fprintf(log_file, "%f\n", p.reward);
    
    fclose(log_file);
    fclose(pos_file);
    return 0;
}
