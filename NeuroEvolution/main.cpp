//
//  main.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include <iostream>
#include "population.h"

ne_population population;

#define time_limit 1000

#define time_step 0.01f

std::ofstream log_file;
std::ofstream pos_file;

int gens = 32;
int pop = 256;

int trials = 8;

struct Obj
{
    static const int input_size;
    static const int output_size;
    
    double reward;
    
    virtual void run(ne_genome* gen) = 0;
};

struct Pendulum : public Obj
{
    
    static const int input_size = 5;
    static const int output_size = 1;
    
    double x;
    double vx;
    double a;
    double va;
    
    double g;
    double m_c;
    double m_p;
    double m;
    double l;
    double f;
    double b;
    
    double xt = 2.4;
    
    Pendulum() {
        g = 9.8;
        m_c = 0.5;
        m_p = 0.5;
        m = m_c + m_p;
        l = 0.6;
        f = 20.0;
        b = 0.1;
    }
    
    void reset() {
        double stdev = 0.1;
        
        x = gaussian_random() * stdev;
        vx = gaussian_random() * stdev;
        a = gaussian_random() * stdev + M_PI;
        va = gaussian_random() * stdev;
    }
    
    void step(double dt, ne_genome* gen) {
        ne_node* inputs = gen->inputs();
        ne_node* outputs = gen->outputs();
        
        double c = cos(a);
        double s = sin(a);
        
        inputs[0].value = vx;
        inputs[1].value = x;
        inputs[2].value = c;
        inputs[3].value = s;
        inputs[4].value = va;
        
        population.compute(gen);
        
        double action = outputs[0].value;
        
        action = action < -1.0 ? -1.0 : (action > 1.0 ? 1.0 : action);
        
        action *= f;
        
        double va2 = va * va;
        double sc = s * c;
        double c2 = c * c;

        double vvx = (-2.0 * m_p * l * va2 * s + 3.0 * m_p * g * sc + 4.0 * action - 4.0 * b * vx) / (4.0 * m - 3.0 * m_p * c2);
        double vva = (-3.0 * m_p * l * va2 * sc + 6.0 * m * g * s + 6.0 * (action - b * vx) * c) / (4.0 * l * m - 3.0 * m_p * l * c2);
        
        vx = vx + vvx * dt;
        va = va + vva * dt;
        
        x = x + vx * dt;
        a = a + va * dt;
        
        double q = x / xt;
        
        q = q > 1.0 ? 1.0 : q;
        q = q < -1.0 ? -1.0 : q;
        
        reward += 0.5 * (cos(a) + 1.0) * (cos(q * M_PI * 0.5));
    }
    
    void run(ne_genome* g) {
        for(int q = 0; q < trials; ++q) {
            reset();
            g->flush();
            
            for(int i = 0; i < time_limit; ++i) {
                step(time_step, g);
            }
        }
        
        reward /= (double) trials;
    }
    
};

struct XOR : public Obj
{
    static const int input_size = 2;
    static const int output_size = 1;
    
    void run(ne_genome* gen) {
        for(int a = 0; a < 2; ++a) {
            for(int b = 0; b < 2; ++b) {
                int c = a ^ b;
                
                ne_node* inputs = gen->inputs();
                ne_node* outputs = gen->outputs();
                
                inputs[0].value = a;
                inputs[1].value = b;
                
                population.compute(gen);
                
                double d = outputs[0].value - c;
                reward += 1.0 - d * d;
            }
        }
    }
};

typedef Pendulum obj_type;

std::vector<obj_type> objs(pop);

void initialize() {
    ne_params params;
    params.input_size = obj_type::input_size;
    params.output_size = obj_type::output_size;
    params.population = pop;
    population.reset(params);
}

int main(int argc, const char * argv[]) {
    log_file.open("log.txt");
    pos_file.open("pos.txt");

    initialize();
    
    ne_genome* best = nullptr;
    
    log_file << "Population: " << pop << std::endl;
    
    log_file.flush();
    
    for(int n = 0; n < gens; ++n) {
        for(int i = 0; i < pop; ++i) {
            objs[i].reward = 0.0;
            
            objs[i].run(population[i]);
            
            population[i]->fitness = objs[i].reward;
        }
        
        log_file << std::endl << std::endl << std::endl;
        
        log_file << "Generation: " << n << std::endl;
        
        best = population.select();
        
        for(int i = 0; i < pop; ++i) {
            log_file << "fitness: " << population[i]->fitness << "  complexity: " << population[i]->complexity() << "  size: " << population[i]->size() << std::endl;
        }
        
        log_file << std::endl << std::endl << std::endl;
        
        for(ne_species* sp : population.species) {
            log_file << "Species: " << sp->avg_fitness << "  offsprings: " << sp->offsprings << "  size: " << sp->genomes.size() << std::endl;
        }
        
        log_file << std::endl << std::endl << std::endl;
        
        log_file << "fitness: " << best->fitness << "  complexity: " << best->complexity() << "  size: " << best->size() << std::endl;
        
        log_file << std::endl << std::endl << std::endl;
        
        log_file.flush();
        
        population.reproduce();
    }
    
    Pendulum p;
    p.reward = 0.0;
    
    p.reset();
    best->flush();
    
    for(int i = 0; i < time_limit; ++i) {
        p.step(time_step, best);
        
        pos_file << p.x << ", " << p.a << "," << std::endl;
    }
    
    log_file << p.reward << std::endl;
        
    log_file.close();
    pos_file.close();
    return 0;
}
