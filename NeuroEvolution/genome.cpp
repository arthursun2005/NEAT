//
//  genome.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/15/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "genome.h"

void ne_genome::reset(size_t inputs, size_t outputs) {
    input_size = inputs + 1;
    output_size = outputs;
    
    nodes.resize(input_size + output_size);
    
    _destory_genes();
    
    for(size_t i = 0; i < input_size; ++i) {
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

void ne_genome::initialize(ne_innov_map* map, size_t* innov) {
    for(size_t i = 0; i < input_size; ++i) {
        for(size_t j = 0; j < output_size; ++j) {
            size_t q = input_size + j;
            ne_gene* gene = new ne_gene(i, q, get_innov(map, innov, ne_innov(i, q)));
            gene->enabled = true;
            gene->link->weight = gaussian_random();
            insert(gene);
        }
    }
}

bool ne_genome::done() const {
    if(activations >= genes.size())
        return true;
    
    for(size_t i = 0; i != output_size; ++i) {
        if(nodes[input_size + i].activations == 0)
            return false;
    }
    
    return true;
}

void ne_genome::flush() {
    size_t size = nodes.size();
    
    for(size_t i = input_size; i != size; ++i) {
        nodes[i].activations = 0;
        nodes[i].value = 0.0;
    }
    
    activations = 0;
}

void ne_genome::_compute(const ne_params& params) {
    size_t size = nodes.size();
    size_t qf = input_size + output_size;
    
    size_t n = 0;
    
    while(n != params.activations) {
        for(size_t i = input_size; i < size; ++i) {
            nodes[i].computed = false;
            nodes[i].sum = 0.0;
        }
        
        for(ne_gene* gene : genes) {
            if(gene->enabled) {
                if(nodes[gene->link->i].activations != 0) {
                    nodes[gene->link->j].computed = true;
                    nodes[gene->link->j].sum += nodes[gene->link->i].value * gene->link->weight;
                }
            }
        }
        
        for(size_t i = input_size; i < size; ++i) {
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

size_t ne_genome::create_node() {
    ne_node node;
    size_t i = nodes.size();
    nodes.push_back(node);
    return i;
}

void ne_genome::insert(ne_gene *gene) {
    set.insert(ne_innov(*gene->link));
    
    std::vector<ne_gene*>::iterator end = genes.end();
    
    while(end-- != genes.begin()) {
        if(gene->innov > (*end)->innov) break;
    }
    
    ++end;
    genes.insert(end, gene);
}

void ne_genome::mutate_weights(const ne_params& params) {
    for(ne_gene* gene : genes) {
        if(ne_random() < params.weights_reset_prob)
            gene->link->weight = gaussian_random();
        else
            gene->link->weight += ne_random(-params.weights_mutation_power, params.weights_mutation_power);
    }
}

void ne_genome::mutate_topology_add_node(ne_innov_map *map, size_t *innov, const ne_params& params) {
    size_t gs = genes.size();
    
    if(gs != 0) {
        for(size_t n = 0; n != params.timeout; ++n) {
            ne_gene* gene = genes[rand64() % gs];
            
            if(!gene->enabled) continue;
            
            size_t node = create_node();
            
            gene->enabled = false;
            
            {
                ne_gene* gene1 = new ne_gene(gene->link->i, node, get_innov(map, innov, ne_innov(gene->link->i, node)));

                gene1->link->weight = 1.0;
                gene1->enabled = true;
                
                insert(gene1);
            }
            
            {
                ne_gene* gene2 = new ne_gene(node, gene->link->j, get_innov(map, innov, ne_innov(node, gene->link->j)));
                
                gene2->link->weight = gene->link->weight;
                gene2->enabled = true;
                
                insert(gene2);
            }
            
            break;
        }
    }
}

void ne_genome::mutate_topology_add_gene(ne_innov_map *map, size_t *innov, const ne_params& params) {
    size_t size = nodes.size();
    
    for(size_t n = 0; n != params.timeout; ++n) {
        size_t i = rand64() % size;
        size_t j = input_size + (rand64() % (size - input_size));
        
        ne_innov in(i, j);
        
        auto it = set.find(ne_innov(in));
        if(it != set.end()) {
            continue;
        }else{
            ne_gene* gene = new ne_gene(i, j, get_innov(map, innov, in));
            
            gene->link->weight = gaussian_random();
            gene->enabled = true;
            
            insert(gene);
        }
        
        break;
    }
}

void ne_genome::mutate_toggle_gene_enable(size_t times) {
    size_t gs = genes.size();
    
    if(gs != 0) {
        for(size_t n = 0; n < times; ++n) {
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
    
    while(itA != A->genes.end() || itB != B->genes.end()) {
        size_t i, j, innov;
        double weight;
        bool enabled;
        
        if(itA == A->genes.end()) {
            i = (*itB)->link->i;
            j = (*itB)->link->j;
            innov = (*itB)->innov;
            weight = (*itB)->link->weight;
            enabled = (*itB)->enabled;
            ++itB;
        }else if(itB == B->genes.end()) {
            i = (*itA)->link->i;
            j = (*itA)->link->j;
            innov = (*itA)->innov;
            weight = (*itA)->link->weight;
            enabled = (*itA)->enabled;
            ++itA;
        }else if((*itA)->innov == (*itB)->innov) {
            i = (*itA)->link->i;
            j = (*itA)->link->j;
            innov = (*itA)->innov;
            
            if(avg) weight = ((*itA)->link->weight + (*itB)->link->weight) * 0.5;
            else weight = (rand32() & 1) ? (*itA)->link->weight : (*itB)->link->weight;
            
            if((*itA)->enabled && (*itB)->enabled) {
                enabled = true;
            }else if((*itA)->enabled) {
                enabled = ne_random() < params.disable_inheritance ? false : true;
            }else if((*itB)->enabled) {
                enabled = ne_random() < params.disable_inheritance ? false : true;
            }else{
                enabled = false;
            }
            
            ++itA;
            ++itB;
        }else if((*itA)->innov < (*itB)->innov) {
            i = (*itA)->link->i;
            j = (*itA)->link->j;
            innov = (*itA)->innov;
            weight = (*itA)->link->weight;
            enabled = (*itA)->enabled;
            ++itA;
        }else{
            i = (*itB)->link->i;
            j = (*itB)->link->j;
            innov = (*itB)->innov;
            weight = (*itB)->link->weight;
            enabled = (*itB)->enabled;
            ++itB;
        }
        
        ne_innov in(i, j);
        auto it = C->set.find(in);
        if(it == C->set.end()) {
            ne_gene* gene = new ne_gene(i, j, innov);
            gene->link->weight = weight;
            gene->enabled = enabled;
            C->insert(gene);
        }
    }
}

double ne_distance(const ne_genome *A, const ne_genome *B, const ne_params& params) {
    std::vector<ne_gene*>::const_iterator itA, itB;
    
    itA = A->genes.begin();
    itB = B->genes.begin();
    
    size_t miss = 0;
    size_t n = 0;
    double W = 0.0;
    
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
        
        if((*itA)->innov == (*itB)->innov) {
            double d = (*itA)->link->weight - (*itB)->link->weight;
            W += d * d;
            ++n;
            
            ++itA;
            ++itB;
        }else if((*itA)->innov < (*itB)->innov) {
            ++miss;
            ++itA;
        }else{
            ++miss;
            ++itB;
        }
    }
    
    return miss / (double) (n + miss) + params.weights_power * sqrt(W / (double) n);
}
