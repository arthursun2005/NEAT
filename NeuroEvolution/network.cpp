//
//  network.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "network.h"
#include <stack>

void set_innov(ne_innov_map* map, size_t* innov, ne_link* link) {
    ne_innov i(*link);
    auto it = map->find(i);
    
    if(it != map->end()) {
        link->innov = it->second;
    }else{
        link->innov = (*innov)++;
        map->insert({i, link->innov});
    }
}

void ne_network::clear() {
    set.clear();
    
    for(ne_link* link : links)
        delete link;
    
    links.clear();
}

void ne_network::reset(size_t inputs, size_t outputs) {
    input_size = inputs + 1;
    output_size = outputs;
    
    nodes.resize(input_size + output_size);
    
    clear();
    
    age = 0;
    
    for(size_t i = 0; i < input_size; ++i) {
        nodes[i].acts = 1;
        nodes[i].value = gaussian_random();
    }
    
    killed = false;
}

void ne_network::initialize(ne_innov_map* map, size_t* innov) {
    for(size_t i = 0; i < input_size; ++i) {
        for(size_t j = 0; j < output_size; ++j) {
            ne_link* link = new ne_link();
            link->i = i;
            link->j = input_size + j;
            link->enabled = true;
            link->weight = gaussian_random();
            set_innov(map, innov, link);
            insert(link);
        }
    }
}

bool ne_network::outputs_off() const {
    size_t size = nodes.size();
    
    for(size_t i = input_size; i < size; ++i) {
        if(nodes[i].acts < 2)
            return true;
    }
    
    return false;
}

void ne_network::compute() {
    size_t size = nodes.size();
    
    for(size_t i = input_size; i < size; ++i) {
        nodes[i].acts = 0;
    }
    
    size_t qf = input_size + output_size;
    
    size_t n = 0;
    size_t lm = links.size();
    
    while(outputs_off() && n != lm) {
        for(size_t i = input_size; i < size; ++i) {
            nodes[i].computed = false;
            nodes[i].sum = 0.0f;
        }
        
        for(ne_link* link : links) {
            if(link->enabled) {
                if(nodes[link->i].acts != 0) {
                    nodes[link->j].computed = true;
                    nodes[link->j].sum += nodes[link->i].value * link->weight;
                }
            }
        }
        
        for(size_t i = input_size; i < size; ++i) {
            if(nodes[i].computed) {
                ++nodes[i].acts;
                
                if(i >= qf)
                    nodes[i].value = ne_func(nodes[i].sum);
                else
                    nodes[i].value = nodes[i].sum;
            }
        }
        
        ++n;
    }
}

size_t ne_network::create_node() {
    ne_node node;
    size_t i = nodes.size();
    nodes.push_back(node);
    return i;
}

void ne_network::insert(ne_link *link) {
    set.insert(ne_innov(*link));
    
    std::vector<ne_link*>::iterator end = links.end();
    
    while(end-- != links.begin()) {
        if(link->innov > (*end)->innov) break;
    }
    
    ++end;
    links.insert(end, link);
}

void ne_network::mutate_weights(const ne_params& params) {
    for(ne_link* link : links) {
        if(ne_random() < params.weights_reset_prob)
            link->weight = gaussian_random();
        else
            link->weight += ne_random(-params.weights_mutation_power, params.weights_mutation_power);
    }
    
    ne_node& node = nodes[input_size - 1];
    
    if(ne_random() <params. weights_reset_prob)
        node.value = gaussian_random();
    else
        node.value += ne_random(-params.weights_mutation_power, params.weights_mutation_power);
}

void ne_network::mutate_topology_add_node(ne_innov_map *map, size_t *innov, const ne_params& params) {
    size_t ls = links.size();
    
    if(ls != 0) {
        ne_link* link = links[rand64() % ls];
        
        if(!link->enabled) return;
        
        size_t node = create_node();
        
        link->enabled = false;
        
        {
            ne_link* link1 = new ne_link();
            link1->i = link->i;
            link1->j = node;
            
            set_innov(map, innov, link1);
            
            link1->weight = 1.0f;
            link1->enabled = true;
            
            insert(link1);
        }
        
        {
            ne_link* link2 = new ne_link();
            link2->i = node;
            link2->j = link->j;
            
            set_innov(map, innov, link2);
            
            link2->weight = link->weight;
            link2->enabled = true;
            
            insert(link2);
        }
    }
}

void ne_network::mutate_topology_add_link(ne_innov_map *map, size_t *innov, const ne_params& params) {
    size_t size = nodes.size();
    
    size_t i = rand64() % size;
    size_t j = input_size + (rand64() % (size - input_size));
    
    ne_link l;
    l.i = i;
    l.j = j;
    
    auto it = set.find(ne_innov(l));
    if(it != set.end()) {
        return;
    }else{
        ne_link* link = new ne_link(l);
        
        set_innov(map, innov, link);
        
        link->weight = gaussian_random();
        link->enabled = true;
        
        insert(link);
    }
}

void ne_network::mutate_toggle_link_enable(size_t times) {
    size_t ls = links.size();
    
    if(ls != 0) {
        for(size_t n = 0; n < times; ++n) {
            ne_link* link = links[rand64() % ls];
            link->enabled = !link->enabled;
        }
    }
}

ne_network::ne_network(const ne_network& network) {
    *this = network;
}

ne_network& ne_network::operator = (const ne_network &network) {
    reset(network.input_size - 1, network.output_size);
    
    nodes = network.nodes;
    
    fitness = network.fitness;
    
    for(ne_link* link : network.links) {
        insert(new ne_link(*link));
    }
    
    return *this;
}

void ne_network::crossover(const ne_network* A, const ne_network* B, ne_network* C, const ne_params& params) {
    C->reset(A->input_size - 1, B->output_size);
    
    bool avg = ne_random() < params.mate_avg_prob;
    
    {
        size_t i = C->input_size - 1;
        if(avg) C->nodes[i].value = (A->nodes[i].value + B->nodes[i].value) * 0.5f;
        else C->nodes[i].value = (rand32() & 1) ? A->nodes[i].value : B->nodes[i].value;
    }
    
    C->fitness = (A->fitness + B->fitness) * 0.5f;
    
    std::vector<ne_link*>::const_iterator itA, itB;
    
    itA = A->links.begin();
    itB = B->links.begin();
    
    while(itA != A->links.end() || itB != B->links.end()) {
        ne_link link;
        
        if(itA == A->links.end()) {
            link = **itB;
            ++itB;
        }else if(itB == B->links.end()) {
            link = **itA;
            ++itA;
        }else if((*itA)->innov == (*itB)->innov) {
            link = **itA;
            
            if(avg) link.weight = ((*itA)->weight + (*itB)->weight) * 0.5f;
            else link.weight = (rand32() & 1) ? (*itA)->weight : (*itB)->weight;
            
            if((*itA)->enabled && (*itB)->enabled) {
                link.enabled = true;
            }else if((*itA)->enabled) {
                link.enabled = ne_random() < params.disable_inheritance ? false : true;
            }else if((*itB)->enabled) {
                link.enabled = ne_random() < params.disable_inheritance ? false : true;
            }else{
                link.enabled = false;
            }
            
            ++itA;
            ++itB;
        }else if((*itA)->innov < (*itB)->innov) {
            link = **itA;
            ++itA;
        }else{
            link = **itB;
            ++itB;
        }
        
        auto it = C->set.find(ne_innov(link));
        if(it == C->set.end()) {
            C->insert(new ne_link(link));
        }
    }
    
    C->nodes.resize(std::max(A->nodes.size(), B->nodes.size()));
}

float ne_network::distance(const ne_network *A, const ne_network *B, const ne_params& params) {
    std::vector<ne_link*>::const_iterator itA, itB;
    
    itA = A->links.begin();
    itB = B->links.begin();
    
    size_t miss = 0;
    size_t n = 0;
    float W = 0.0f;
    
    while(itA != A->links.end() || itB != B->links.end()) {
        if(itA == A->links.end()) {
            ++miss;
            ++itB;
            continue;
        }
        
        if(itB == B->links.end()) {
            ++miss;
            ++itA;
            continue;
        }
        
        if((*itA)->innov == (*itB)->innov) {
            float d = (*itA)->weight - (*itB)->weight;
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
    
    return miss / (float) (n + miss) + params.weights_power * sqrtf(W / (float) n);
}
    
