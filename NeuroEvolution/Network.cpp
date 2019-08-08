//
//  Network.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "Network.h"
#include <stack>

namespace NE {
    
    size_t get_innov(innov_set* set, size_t* innov, Link* link) {
        auto it = set->find(link);
        
        if(it != set->end()) {
            return (*it)->innov;
        }else{
            set->insert(link);
            return ++(*innov);
        }
    }

    void Network::clear() {
        set.clear();
        
        for(Link* link : links)
            delete link;
        
        links.clear();
    }
    
    void Network::reset(size_t inputs, size_t outputs) {
        nodes.resize(inputs + outputs);
        
        clear();
        
        input_size = inputs;
        output_size = outputs;
        
        age = 0;
    }
    
    bool Network::outputs_off() const {
        for(size_t i = 0; i < output_size; ++i) {
            if(nodes[input_size + i].acts == 0)
                return true;
        }
        
        return false;
    }
    
    void Network::compute() {
        size_t size = nodes.size();
        
        for(size_t i = 0; i < input_size; ++i) {
            nodes[i].value = nodes[i].function(nodes[i].value);
            nodes[i].acts = 1;
        }
        
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
            
            for(Link* link : links) {
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
                        nodes[i].value = nodes[i].function(nodes[i].sum);
                    else
                        nodes[i].value = nodes[i].sum;
                }
            }
            
            ++n;
        }
    }
    
    size_t Network::create_node() {
        Node node;
        
        size_t i = nodes.size();
        
        nodes.push_back(node);
        
        return i;
    }
    
    void Network::insert(Link *link) {
        set.insert(link);
        
        std::vector<Link*>::iterator end = links.end();
        
        while(end-- != links.begin()) {
            if(link->innov > (*end)->innov) break;
        }
        
        ++end;
        links.insert(end, link);
    }
    
    void Network::disable(Link *link) {
        link->enabled = false;
    }
    
    void Network::mutate(innov_set *set, size_t *innov) {
        size_t size = nodes.size();
        size_t ls = links.size();
        
        {            
            for(Link* link : links) {
                if(rand64() & 1)
                    link->weight += random() * randposneg();
            }
        }
        
        uint32_t k = rand32() & 0xffff;
        
        if(true) {
            if(k <= 0x3fff && ls != 0) {
                Link* link = links[rand64() % ls];
                size_t node = create_node();
                
                if(!link->enabled) return;
                
                link->enabled = false;
                
                {
                    Link* link1 = new Link();
                    link1->i = link->i;
                    link1->j = node;
                    
                    link1->innov = get_innov(set, innov, link1);
                    
                    link1->weight = gaussian_random();
                    link1->enabled = true;
                    
                    insert(link1);
                }
                
                {
                    Link* link2 = new Link();
                    link2->i = node;
                    link2->j = link->j;
                    
                    link2->innov = get_innov(set, innov, link2);
                    
                    link2->weight = gaussian_random();
                    link2->enabled = true;
                    
                    insert(link2);
                }
            }else{
                if(rand32() & 1 || ls == 0) {
                    size_t i = rand64() % size;
                    size_t j = input_size + (rand64() % (size - input_size));
                    
                    Link l;
                    l.i = i;
                    l.j = j;
                    
                    if(this->set.find(&l) != this->set.end() || i == j) {
                        return;
                    }
                    
                    Link* link = new Link(l);
                    
                    link->innov = get_innov(set, innov, link);
                    
                    link->weight = gaussian_random();
                    link->enabled = true;
                    
                    insert(link);
                }else{
                    disable(links[rand64() % ls]);
                }
            }
        }
    }
    
    Network::Network(const Network& network) {
        *this = network;
    }
    
    Network& Network::operator = (const Network &network) {
        reset(network.input_size, network.output_size);
        
        nodes = network.nodes;
        
        fitness = network.fitness;
        
        age = network.age;
                
        for(Link* link : network.links) {
            insert(new Link(*link));
            ++link;
        }
        
        return *this;
    }
    
    void Network::crossover(Network* A, Network* B, Network* C) {
        C->reset(A->input_size, B->output_size);
        
        size_t size = std::max(A->nodes.size(), B->nodes.size());
        
        C->nodes.resize(size);
        
        C->fitness = (A->fitness + B->fitness) * 0.5f;
        C->age = 0;
        
        std::vector<Link*>::iterator itA, itB;
        
        itA = A->links.begin();
        itB = B->links.begin();
        
        while(itA != A->links.end() || itB != B->links.end()) {
            if(itA == A->links.end()) {
                C->insert(new Link(**itB));
                ++itB;
                continue;
            }
            
            if(itB == B->links.end()) {
                C->insert(new Link(**itA));
                ++itA;
                continue;
            }
            
            if((*itA)->innov == (*itB)->innov) {
                Link* link = new Link(**itA);
                link->weight = ((*itA)->weight + (*itB)->weight) * 0.5f;
                
                if((*itA)->enabled && (*itB)->enabled) {
                    link->enabled = (rand32() & 0xff) <= 0x3f ? false : true;
                }else if((*itA)->enabled) {
                    link->enabled = (rand32() & 0xff) <= 0x7f ? true : false;
                }else if((*itB)->enabled) {
                    link->enabled = (rand32() & 0xff) <= 0x7f ? false : true;
                }else{
                    link->enabled = (rand32() & 0xff) <= 0x3f ? true : false;
                }
                
                C->insert(link);
                
                ++itA;
                ++itB;
            }else if((*itA)->innov < (*itB)->innov) {
                C->insert(new Link(**itA));
                ++itA;
            }else{
                C->insert(new Link(**itB));
                ++itB;
            }
        }
    }
    
    float Network::distance_topology(Network *A, Network *B) {
        std::vector<Link*>::iterator itA, itB;
        
        itA = A->links.begin();
        itB = B->links.begin();
        
        size_t miss = 0;
        size_t n = 0;
        
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
        
        size_t N = miss + n;
        return (N == 0 ? 0.0f : miss / (float) N);
    }
    
    float Network::distance_weights(Network *A, Network *B) {
        std::vector<Link*>::iterator itA, itB;
        
        itA = A->links.begin();
        itB = B->links.begin();
        
        float W = 0.0f;
        
        size_t n = 0;
        
        while(itA != A->links.end() || itB != B->links.end()) {
            if(itA == A->links.end()) {
                ++itB;
                continue;
            }
            
            if(itB == B->links.end()) {
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
                ++itA;
            }else{
                ++itB;
            }
        }
        
        return (n == 0 ? 0.0f : sqrtf(W / (float) n));
    }
    
}
