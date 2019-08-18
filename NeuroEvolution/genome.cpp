//
//  genome.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/15/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "genome.h"

void ne_genome::reset(uint64 inputs, uint64 outputs) {
    input_size = inputs + 1;
    output_size = outputs;
    
    nodes.resize(input_size + output_size);
    
    _destory_genes();
    
    for(uint64 i = 0; i < input_size; ++i) {
        nodes[i].activations = 1;
    }
    
    nodes[input_size - 1].value = 1.0;
}

void ne_genome::_destory_genes() {
    set.clear();
    
    for(ne_gene* gene : genes)
        delete gene;
    
    genes.clear();
}

void ne_genome::initialize(ne_innovation_map* map, uint64* innovation) {
    for(uint64 i = 0; i < input_size; ++i) {
        for(uint64 j = 0; j < output_size; ++j) {
            uint64 q = input_size + j;
            ne_gene* gene = new ne_gene(i, q);
            gene->innovation = get_innovation(map, innovation, *gene);
            gene->enabled = true;
            gene->weight = gaussian_random();
            insert(gene);
        }
    }
}

bool ne_genome::done() const {
    if(activations >= genes.size())
        return true;
    
    for(uint64 i = 0; i != output_size; ++i) {
        if(nodes[input_size + i].activations == 0)
            return false;
    }
    
    return true;
}

void ne_genome::flush() {
    uint64 size = nodes.size();
    
    for(uint64 i = input_size; i != size; ++i) {
        nodes[i].activations = 0;
        nodes[i].value = 0.0;
    }
    
    activations = 0;
}

void ne_genome::_compute(const ne_params& params) {
    uint64 size = nodes.size();
    uint64 qf = input_size + output_size;
    
    uint64 n = 0;
    
    while(n != params.activations) {
        for(uint64 i = input_size; i < size; ++i) {
            nodes[i].computed = false;
            nodes[i].sum = 0.0;
        }
        
        for(ne_gene* gene : genes) {
            if(gene->enabled) {
                if(nodes[gene->i].activations != 0) {
                    nodes[gene->j].computed = true;
                    nodes[gene->j].sum += nodes[gene->i].value * gene->weight;
                }
            }
        }
        
        for(uint64 i = input_size; i < size; ++i) {
            if(nodes[i].computed) {
                ++nodes[i].activations;
                
                if(i >= qf)
                    nodes[i].value = ne_func(nodes[i].sum);
                else
                    nodes[i].value = nodes[i].sum;
            }
        }
        
        ++n;
    }
    
    activations += params.activations;
}

uint64 ne_genome::create_node() {
    ne_node node;
    uint64 i = nodes.size();
    nodes.push_back(node);
    return i;
}

void ne_genome::insert(ne_gene *gene) {
    set.insert(*gene);
    
    std::vector<ne_gene*>::iterator end = genes.end();
    
    while(end-- != genes.begin()) {
        if(gene->innovation > (*end)->innovation) break;
    }
    
    ++end;
    genes.insert(end, gene);
}

void ne_genome::mutate_weights(const ne_params& params) {
    for(ne_gene* gene : genes) {
        if(ne_random() < params.weights_reset_prob)
            gene->weight = gaussian_random();
        else
            gene->weight += ne_random(-params.weights_mutation_power, params.weights_mutation_power);
    }
}

void ne_genome::mutate_topology_add_node(ne_innovation_map *map, uint64 *innovation, const ne_params& params) {
    uint64 gs = genes.size();
    
    if(gs != 0) {
        for(uint64 n = 0; n != params.timeout; ++n) {
            ne_gene* gene = genes[rand64() % gs];
            
            if(!gene->enabled) continue;
            
            uint64 node = create_node();
            
            gene->enabled = false;
            
            {
                ne_gene* gene1 = new ne_gene(gene->i, node);
                gene1->innovation = get_innovation(map, innovation, *gene1);

                gene1->weight = 1.0;
                gene1->enabled = true;
                
                insert(gene1);
            }
            
            {
                ne_gene* gene2 = new ne_gene(node, gene->j);
                gene2->innovation = get_innovation(map, innovation, *gene2);
                
                gene2->weight = gene->weight;
                gene2->enabled = true;
                
                insert(gene2);
            }
            
            break;
        }
    }
}

void ne_genome::mutate_topology_add_gene(ne_innovation_map *map, uint64 *innovation, const ne_params& params) {
    uint64 size = nodes.size();
    
    for(uint64 n = 0; n != params.timeout; ++n) {
        uint64 i = rand64() % size;
        uint64 j = input_size + (rand64() % (size - input_size));
        
        ne_innovation_set::iterator it = set.find(ne_innovation(i, j));
        if(it != set.end()) {
            continue;
        }else{
            ne_gene* gene = new ne_gene(i, j);
            gene->innovation = get_innovation(map, innovation, *gene);
            
            gene->weight = gaussian_random();
            gene->enabled = true;
            
            insert(gene);
        }
        
        break;
    }
}

void ne_genome::mutate_toggle_gene_enable(uint64 times) {
    uint64 gs = genes.size();
    
    if(gs != 0) {
        for(uint64 n = 0; n < times; ++n) {
            ne_gene* gene = genes[rand64() % gs];
            gene->enabled = !gene->enabled;
        }
    }
}

ne_genome::ne_genome(const ne_genome& genome) {
    *this = genome;
}

ne_genome& ne_genome::operator = (const ne_genome &genome) {
    reset(genome.input_size - 1, genome.output_size);
    
    nodes = genome.nodes;
    
    fitness = genome.fitness;
    
    for(ne_gene* gene : genome.genes) {
        insert(new ne_gene(*gene));
    }
    
    return *this;
}

void ne_crossover(const ne_genome* A, const ne_genome* B, ne_genome* C, const ne_params& params) {
    C->reset(A->input_size - 1, B->output_size);
    
    C->nodes.resize(std::max(A->nodes.size(), B->nodes.size()));
    
    bool avg = ne_random() < params.mate_avg_prob;
    
    C->fitness = (A->fitness + B->fitness) * 0.5;
    
    std::vector<ne_gene*>::const_iterator itA, itB;
    
    itA = A->genes.begin();
    itB = B->genes.begin();
    
    bool a_over_b = A->adjusted_fitness > B->adjusted_fitness;
    
    ne_gene gene;
    bool skip;
    
    while(itA != A->genes.end() || itB != B->genes.end()) {
        if(itA == A->genes.end()) {
            gene = **itB;
            skip = a_over_b;
            ++itB;
        }else if(itB == B->genes.end()) {
            gene = **itA;
            skip = !a_over_b;
            ++itA;
        }else if((*itA)->innovation == (*itB)->innovation) {
            gene = **itA;
            
            if(avg) gene.weight = ((*itA)->weight + (*itB)->weight) * 0.5;
            else gene.weight = (rand32() & 1) ? (*itA)->weight : (*itB)->weight;
            
            if((*itA)->enabled && (*itB)->enabled) {
                gene.enabled = true;
            }else if((*itA)->enabled) {
                gene.enabled = ne_random() < params.disable_inheritance ? false : true;
            }else if((*itB)->enabled) {
                gene.enabled = ne_random() < params.disable_inheritance ? false : true;
            }else{
                gene.enabled = false;
            }
            
            skip = false;
            
            ++itA;
            ++itB;
        }else if((*itA)->innovation < (*itB)->innovation) {
            gene = **itA;
            skip = !a_over_b;
            ++itA;
        }else{
            gene = **itB;
            skip = a_over_b;
            ++itB;
        }
        
        if(skip) continue;
        
        ne_innovation_set::iterator it = C->set.find(gene);
        if(it == C->set.end()) {
            C->insert(new ne_gene(gene));
        }
    }
}

float64 ne_distance(const ne_genome *A, const ne_genome *B, const ne_params& params) {
    std::vector<ne_gene*>::const_iterator itA, itB;
    
    itA = A->genes.begin();
    itB = B->genes.begin();
    
    uint64 miss = 0;
    uint64 n = 0;
    float64 W = 0.0;
    
    while(itA != A->genes.end() || itB != B->genes.end()) {
        if(itA == A->genes.end()) {
            ++miss;
            ++itB;
            continue;
        }
        
        if(itB == B->genes.end()) {
            ++miss;
            ++itA;
            continue;
        }
        
        if((*itA)->innovation == (*itB)->innovation) {
            float64 d = (*itA)->weight - (*itB)->weight;
            W += d * d;
            ++n;
            
            ++itA;
            ++itB;
        }else if((*itA)->innovation < (*itB)->innovation) {
            ++miss;
            ++itA;
        }else{
            ++miss;
            ++itB;
        }
    }
    
    return miss / (float64) (n + miss) + params.weights_power * sqrt(W / (float64) n);
}
