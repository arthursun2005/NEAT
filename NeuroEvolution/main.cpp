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

int gens = 64;
ne_params params;

struct Obj
{
    static const uint64 input_size;
    static const uint64 output_size;
    
    double fitness;
    
    virtual void run(ne_genome* gen) = 0;
};

struct Pendulum : public Obj
{
    static const uint64 input_size = 5;
    static const uint64 output_size = 1;
    
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
        f = 10.0;
        b = 0.1;
    }
    
    void reset() {
        double stdev = 0.1;
        
        x = gaussian_random() * stdev;
        vx = gaussian_random() * stdev;
        a = gaussian_random() * stdev + M_PI;
        va = gaussian_random() * stdev;
    }
    
    void run(ne_genome* gen) {
        fitness = 0.0;
        
        gen->flush();
        
        reset();
        
        ne_node** inputs = gen->inputs();
        ne_node** outputs = gen->outputs();
        
        for(int i = 0; i < time_limit; ++i) {
            double c = cos(a);
            double s = sin(a);

            double action = 0.0;
            
            if((i % 2) == 0) {
                inputs[0]->value = vx;
                inputs[1]->value = x;
                inputs[2]->value = c;
                inputs[3]->value = s;
                inputs[4]->value = va;
                
                gen->compute();
                
                action = outputs[0]->value;
                
                action = action < -1.0 ? -1.0 : (action > 1.0 ? 1.0 : action);
                
                action *= f;
            }
            
            double va2 = va * va;
            double sc = s * c;
            double c2 = c * c;
            
            double vvx = (-2.0 * m_p * l * va2 * s + 3.0 * m_p * g * sc + 4.0 * action - 4.0 * b * vx) / (4.0 * m - 3.0 * m_p * c2);
            double vva = (-3.0 * m_p * l * va2 * sc + 6.0 * m * g * s + 6.0 * (action - b * vx) * c) / (4.0 * l * m - 3.0 * m_p * l * c2);
            
            vx = vx + vvx * time_step;
            va = va + vva * time_step;
            
            x = x + vx * time_step;
            a = a + va * time_step;
            
            if(x < -xt || x > xt)
                break;
            
            fitness += 0.5 * (cos(a) + 1.0);
        }
    }
    
};

struct XOR : public Obj
{
    static const uint64 input_size = 2;
    static const uint64 output_size = 1;
    
    void run(ne_genome* gen) {
        fitness = 0.0;
        
        for(int a = 0; a < 2; ++a) {
            for(int b = 0; b < 2; ++b) {
                gen->flush();
                
                int c = a ^ b;
                
                ne_node** inputs = gen->inputs();
                ne_node** outputs = gen->outputs();
                
                inputs[0]->value = a;
                inputs[1]->value = b;
                
                gen->compute();
                
                double d = outputs[0]->value - c;
                fitness += 1.0 - d * d;
            }
        }
        
        fitness *= 0.25;
    }
};

typedef Pendulum obj_type;

std::vector<obj_type> objs(params.population);

void initialize(const char* file) {
    std::ifstream in;
    in.open(file);
    params.load(in);
    population.reset(params, obj_type::input_size, obj_type::output_size);
    
    objs.resize(params.population);
}

int main(int argc, const char * argv[]) {
    if(argc == 1) {
        std::cout << "No file entered for parameters" << std::endl;
        return 1;
    }
    
    initialize(argv[1]);

    ne_genome* best = nullptr;
    
    std::vector<float64> highs;

    for(int n = 0; n < gens; ++n) {
        for(int i = 0; i < params.population; ++i) {
            objs[i].run(population[i]);
            
            population[i]->fitness = objs[i].fitness;
        }
        
        std::cout << "Generation: " << n << std::endl;
        
        best = population.select();
        
        for(ne_species* sp : population.species) {
            std::cout << "Species: " << sp->avg_fitness << "  offsprings: " << sp->offsprings << "  size: " << sp->genomes.size() << std::endl;
        }
        
        std::cout << "fitness: " << best->fitness << "  gene count: " << best->gene_count() << "  node count: " << best->node_count() << "  activations: " << best->activations << std::endl;
        
        std::cout << std::endl << std::endl << std::endl;
                        
        highs.push_back(best->fitness);
        
        population.reproduce();
    }
    
    std::cout << "Highs: " << std::endl;
    
    for(uint64 i = 0; i < gens; ++i) {
        std::cout << i << "\t" << highs[i] << std::endl;
    }
    
    return 0;
}
