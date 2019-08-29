//
//  genome.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/15/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "genome.h"

void ne_genome::reset(uint64 inputs, uint64 outputs, ne_innovation_set* set, uint64* innovation) {
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
        uint64 q = input_size + i;
        ne_node* node = new ne_node();
        node->id = q;
        insert(node);
    }
    
    for(uint64 i = 0; i < input_size; ++i) {
        for(uint64 j = 0; j < output_size; ++j) {
            uint64 q = input_size + j;
            ne_gene* gene = new ne_gene(nodes[i], nodes[q]);
            
            ne_innovation p(gene);
            p.type = ne_new_gene;
            
            ne_get_innovation(set, innovation, nullptr, &p);
            
            gene->innovation = p.innovation;
            gene->weight = gaussian_random();
            
            insert(gene);
        }
    }
    
    nodes[input_size - 1]->value = 1.0;
    activations = 1;
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

void ne_genome::flush() {
    uint64 size = nodes.size();
    
    for(uint64 i = input_size; i != size; ++i) {
        nodes[i]->activations = 0;
        nodes[i]->value = ne_function(0.0);
    }
}

void ne_genome::compute() {
    uint64 size = nodes.size();

    uint64 q = input_size + output_size;
    
    uint64 n = 0;
    
    while(n != activations) {
        for(uint64 i = input_size; i < size; ++i) {
            nodes[i]->computed = false;
            nodes[i]->sum = 0.0;
        }
        
        for(ne_gene* gene : genes) {
            if(gene->i->activations != 0) {
                gene->j->computed = true;
                gene->j->sum += gene->i->value * gene->weight;
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
    set.insert(gene);
    
    std::vector<ne_gene*>::iterator end = genes.end();
    
    while(end-- != genes.begin()) {
        if(gene->innovation > (*end)->innovation) break;
    }
    
    ++end;
    genes.insert(end, gene);
}

ne_node* ne_genome::find_node(ne_node *n) {
    ne_nodes_map::iterator itA = nodes_map.find(n->id);
    
    if(itA == nodes_map.end()) {
        ne_node* node = new ne_node(*n);
        insert(node);
        return node;
    }else{
        return itA->second;
    }
}

void ne_genome::mutate_weights(const ne_params& params) {
    for(ne_gene* gene : genes) {
        if(gene->weight != 0.0 && random(0.0, 1.0) < params.weights_mutation_rate) {
            do {
                gene->weight += random(-params.weights_mutation_power, params.weights_mutation_power);
            } while (gene->weight == 0.0);
        }
    }
}

void ne_genome::mutate_add_node(ne_innovation_set *set, uint64 *innovation, uint64* node_ids, const ne_params& params) {
    uint64 gs = genes.size();
    
    if(gs != 0) {
        for(uint64 n = 0; n != params.timeout; ++n) {
            ne_gene* gene = genes[rand64() % gs];
            
            if(gene->weight == 0.0) continue;
            
            if(gene->i->id == input_size - 1) continue;
            
            ne_innovation p(gene);
            p.type = ne_new_node;
            
            ne_get_innovation(set, innovation, node_ids, &p);
            
            ne_node* node = new ne_node();
            node->id = p.id;
            insert(node);
            
            {
                ne_gene* gene1 = new ne_gene(gene->i, node);
                
                gene1->innovation = p.innovation;
                gene1->weight = 1.0;
                
                insert(gene1);
            }
            
            {
                ne_gene* gene2 = new ne_gene(node, gene->j);
                
                gene2->innovation = p.innovation + 1;
                gene2->weight = gene->weight;
                
                insert(gene2);
            }
            
            gene->weight = 0.0;
            
            break;
        }
    }
}

void ne_genome::mutate_add_gene(ne_innovation_set *set, uint64 *innovation, const ne_params& params) {
    uint64 size = nodes.size();
    
    for(uint64 n = 0; n != params.timeout; ++n) {
        ne_gene q(nodes[rand64() % size], nodes[rand64() % size]);
        
        if(q.j == nodes[input_size - 1]) continue;
        
        ne_gene_set::iterator it = this->set.find(&q);
        if(it != this->set.end()) {
            if((*it)->weight == 0.0) {
                (*it)->weight = gaussian_random();
            }else{
                continue;
            }
        }else{
            ne_gene* gene = new ne_gene(q);
            
            ne_innovation p(gene);
            p.type = ne_new_gene;
            
            ne_get_innovation(set, innovation, nullptr, &p);
            
            gene->innovation = p.innovation;
            gene->weight = gaussian_random();
            
            insert(gene);
        }
        
        break;
    }
}

ne_genome::ne_genome(const ne_genome& genome) {
    *this = genome;
}

ne_genome& ne_genome::operator = (const ne_genome &genome) {
    _destory();
    
    input_size = genome.input_size;
    output_size = genome.output_size;
    
    fitness = genome.fitness;
    activations = genome.activations;
    
    eliminated = genome.eliminated;
    
    for(ne_gene* gene : genome.genes) {
        pass_down(new ne_gene(*gene));
    }
    
    return *this;
}

ne_genome* ne_genome::crossover(const ne_genome* A, const ne_genome* B, const ne_params& params) {
    ne_genome* C = new ne_genome();
    
    C->input_size = A->input_size;
    C->output_size = B->output_size;
    
    C->activations = (rand32() & 1) ? A->activations : B->activations;
    
    std::vector<ne_gene*>::const_iterator itA, itB;
    
    itA = A->genes.begin();
    itB = B->genes.begin();
    
    bool a_over_b = A->fitness > B->fitness;
    
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
            
            gene.weight = (rand32() & 1) ? (*itA)->weight : (*itB)->weight;
            
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
        
        ne_gene_set::iterator it = C->set.find(&gene);
        if(it == C->set.end()) {
            C->pass_down(new ne_gene(gene));
        }
    }
    
    return C;
}

float64 ne_genome::distance(const ne_genome *A, const ne_genome *B, const ne_params& params) {
    std::vector<ne_gene*>::const_iterator itA, itB;
    
    itA = A->genes.begin();
    itB = B->genes.begin();
    
    uint64 miss = 0;
    uint64 align = 0;
    
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
            W += fabs((*itA)->weight - (*itB)->weight);
            ++align;
            
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
    
    return miss + (align == 0 ? 0.0 : params.weights_power * W / (float64) align);
}


void ne_genome::write(std::ofstream &out) const {
}

void ne_genome::read(std::ifstream &in) {
}
