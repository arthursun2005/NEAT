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
    
    void set_innov(innov_map* map, size_t* innov, Link* link) {
        auto it = map->find(link);
        
        if(it != map->end()) {
            link->innov = it->second;
        }else{
            link->innov = (*innov)++;
            map->insert({Innov(link), link->innov});
        }
    }

    void Network::clear() {
        set.clear();
        
        for(Link* link : links)
            delete link;
        
        links.clear();
    }
    
    void Network::reset(size_t inputs, size_t outputs) {
        input_size = inputs + 1;
        output_size = outputs;
        
        nodes.resize(input_size + output_size);
        
        clear();
        
        age = 0;
        
        for(size_t i = 0; i < input_size; ++i) {
            nodes[i].acts = 1;
        }
    }
    
    bool Network::outputs_off() const {
        size_t size = nodes.size();
        
        for(size_t i = input_size; i < size; ++i) {
            if(nodes[i].acts < 2)
                return true;
        }
        
        return false;
    }
    
    void Network::compute() {
        size_t size = nodes.size();
        
        for(size_t i = input_size; i < size; ++i) {
            nodes[i].acts = 0;
        }
        
        size_t qf = input_size + output_size;
        
        size_t n = 0;
        size_t lm = 1 + links.size();
        
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
        
        for(Link* link : links) {
            if(link->enabled) {
                if(nodes[link->i].acts != 0 && nodes[link->j].acts != 0) {
                    //link->weight += 0.1f * nodes[link->i].value * nodes[link->j].value;
                    //if(link->weight > 2.0f) link->weight = 2.0f;
                    //if(link->weight < -2.0f) link->weight = -2.0f;
                }
            }
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
    
    void Network::mutate_weights() {
        for(Link* link : links) {
            if(random() < weights_reset_rate)
                link->weight = 2.0f * random() * randposneg();
            
            else if(random() < weights_mutate_rate)
                link->weight += weights_mutation_power * random() * randposneg();
        }
        
        Node& node = nodes[input_size - 1];
        
        if(random() < weights_reset_rate)
            node.value = 2.0f * random() * randposneg();
        
        else if(random() < weights_mutate_rate)
            node.value += weights_mutation_power * random() * randposneg();
    }
    
    void Network::mutate_topology(innov_map *map, size_t *innov) {
        size_t size = nodes.size();
        size_t ls = links.size();
        
        uint32_t k = rand32() & 0xffff;
        
        if(k <= 0x7fff && ls != 0) {
            Link* link = links[rand64() % ls];
            size_t node = create_node();
            
            if(!link->enabled) return;
            
            link->enabled = false;
            
            {
                Link* link1 = new Link();
                link1->i = link->i;
                link1->j = node;
                
                set_innov(map, innov, link1);
                
                link1->weight = 2.0f * random() * randposneg();
                link1->enabled = true;
                
                insert(link1);
            }
            
            {
                Link* link2 = new Link();
                link2->i = node;
                link2->j = link->j;
                
                set_innov(map, innov, link2);
                
                link2->weight = link->weight;
                link2->enabled = true;
                
                insert(link2);
            }
        }else{
            size_t i = rand64() % size;
            size_t j = input_size + (rand64() % (size - input_size));
            
            Link l;
            l.i = i;
            l.j = j;
            
            auto it = set.find(&l);
            if(it != set.end()) {
                (*it)->enabled = !(*it)->enabled;
            }else{
                Link* link = new Link(l);
                
                set_innov(map, innov, link);
                
                link->weight = random() * randposneg() * 2.0f;
                link->enabled = true;
                
                insert(link);
            }
        }
        
        if(ls != 0) {
            Link* link = links[rand64() % ls];
            link->enabled = !link->enabled;
        }
    }
    
    Network::Network(const Network& network) {
        *this = network;
    }
    
    Network& Network::operator = (const Network &network) {
        reset(network.input_size - 1, network.output_size);
        
        nodes = network.nodes;
        
        fitness = network.fitness;
        
        for(Link* link : network.links) {
            insert(new Link(*link));
        }
        
        return *this;
    }
    
    void Network::crossover(const Network* A, const Network* B, Network* C) {
        C->reset(A->input_size - 1, B->output_size);
        
        size_t as = A->nodes.size(), bs = B->nodes.size();
        size_t size = std::max(as, bs);
        
        C->nodes.resize(size);
        
        bool avg = random() < mate_avg_rate;
        
        {
            size_t i = C->input_size - 1;
            if(avg) C->nodes[i].value = (A->nodes[i].value + B->nodes[i].value) * 0.5f;
            else C->nodes[i].value = (rand32() & 1) ? A->nodes[i].value : B->nodes[i].value;
        }
        
        C->fitness = (A->fitness + B->fitness) * 0.5f;
        
        std::vector<Link*>::const_iterator itA, itB;
        
        itA = A->links.begin();
        itB = B->links.begin();
        
        while(itA != A->links.end() || itB != B->links.end()) {
            Link link;
            
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
                    link.enabled = (rand32() & 0xff) <= 0x7f ? true : false;
                }else if((*itB)->enabled) {
                    link.enabled = (rand32() & 0xff) <= 0x7f ? true : false;
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
            
            auto it = C->set.find(&link);
            if(it == C->set.end()) {
                C->insert(new Link(link));
            }
        }
    }
    
    float Network::distance(const Network *A, const Network *B) {
        std::vector<Link*>::const_iterator itA, itB;
        
        itA = A->links.begin();
        itB = B->links.begin();
        
        size_t miss = 0;
        float W = 0.0f;
        
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
        
        size_t N = miss + n;
        return (N == 0 ? 0.0f : miss / (float) N) + (n == 0 ? 0.0f : sqrtf(W / (float) n));
    }
    
}
