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
    
    _destory();
    
    for(uint64 i = 0; i < input_size; ++i) {
        ne_node* node = new ne_node();
        node->id = i;
        node->activations = 1;
        insert(node);
    }
    
    for(uint64 i = 0; i < output_size; ++i) {
        ne_node* node = new ne_node();
        node->id = input_size + i;
        insert(node);
    }
    
    nodes[input_size - 1]->value = 1.0;
}

void ne_genome::_destory() {
    set.clear();
    nodes_map.clear();
    
    for(ne_gene* gene : genes)
        delete gene;
    
    for(ne_node* node : nodes)
        delete node;
    
    genes.clear();
    nodes.clear();
}

void ne_genome::initialize(ne_innovation_map* map, uint64* innovation) {
    for(uint64 i = 0; i < input_size; ++i) {
        for(uint64 j = 0; j < output_size; ++j) {
            uint64 q = input_size + j;
            ne_gene* gene = new ne_gene(nodes[i], nodes[q]);
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
        if(nodes[input_size + i]->activations == 0)
            return false;
    }
    
    return true;
}

void ne_genome::flush() {
    uint64 size = nodes.size();
    
    for(uint64 i = input_size; i != size; ++i) {
        nodes[i]->activations = 0;
    }
    
    activations = 0;
}

void ne_genome::_compute(const ne_params& params) {
    uint64 size = nodes.size();
    uint64 q = input_size + output_size;
    
    uint64 n = 0;
    
    while(n != params.activations) {
        for(uint64 i = input_size; i < size; ++i) {
            nodes[i]->computed = false;
            nodes[i]->sum = 0.0;
        }
        
        for(ne_gene* gene : genes) {
            if(gene->enabled) {
                if(gene->i->activations != 0) {
                    gene->j->computed = true;
                    gene->j->sum += gene->i->value * gene->weight;
                }
            }
        }
        
        for(uint64 i = input_size; i < size; ++i) {
            if(nodes[i]->computed) {
                ++nodes[i]->activations;
                
                if(i >= q)
                    nodes[i]->value = ne_function(nodes[i]->sum);
                else
                    nodes[i]->value = nodes[i]->sum;
            }
        }
        
        ++n;
    }
    
    activations += params.activations;
}

void ne_genome::insert(ne_node *node) {
    nodes_map.insert({node->id, node});
    
    std::vector<ne_node*>::iterator end = nodes.end();
    
    while(end-- != nodes.begin()) {
        if(node->id > (*end)->id) break;
    }
    
    ++end;
    nodes.insert(end, node);
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

void ne_genome::pass_down(ne_gene *gene) {
    ne_nodes_map::iterator itA = nodes_map.find(gene->i->id);
    ne_nodes_map::iterator itB = nodes_map.find(gene->j->id);
    
    if(itA == nodes_map.end()) {
        ne_node* node = new ne_node(*gene->i);
        insert(node);
        gene->i = node;
    }else{
        gene->i = itA->second;
    }
    
    if(itB == nodes_map.end()) {
        ne_node* node = new ne_node(*gene->j);
        insert(node);
        gene->j = node;
    }else{
        gene->j = itB->second;
    }
    
    insert(gene);
}

void ne_genome::mutate_weights(const ne_params& params) {
    for(ne_gene* gene : genes) {
        if(ne_random() < params.weights_mutation_rate)
            gene->weight += ne_random(-params.weights_mutation_power, params.weights_mutation_power);
    }
}

void ne_genome::mutate_topology_add_node(ne_innovation_map *map, uint64 *innovation, uint64* node_ids, const ne_params& params) {
    uint64 gs = genes.size();
    
    if(gs != 0) {
        for(uint64 n = 0; n != params.timeout; ++n) {
            ne_gene* gene = genes[rand64() % gs];
            
            if(!gene->enabled) continue;
            
            ne_node* node = new ne_node();
            node->id = (*node_ids)++;
            insert(node);
            
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
            ne_gene* gene = new ne_gene(nodes[i], nodes[j]);
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
    input_size = genome.input_size;
    output_size = genome.output_size;
    
    _destory();
    
    fitness = genome.fitness;
    
    for(ne_gene* gene : genome.genes) {
        pass_down(new ne_gene(*gene));
    }
    
    return *this;
}

ne_genome* ne_crossover(const ne_genome* A, const ne_genome* B, const ne_params& params) {
    ne_genome* C = new ne_genome();
    
    C->input_size = A->input_size;
    C->output_size = B->output_size;
    
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
            C->pass_down(new ne_gene(gene));
        }
    }
    
    return C;
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
    
    return miss + (n != 0 ? params.weights_power * sqrt(W / (float64) n) : 0.0);
}



void ne_genome::write(std::ofstream &out) const {
}

void ne_genome::read(std::ifstream &in) {
}
