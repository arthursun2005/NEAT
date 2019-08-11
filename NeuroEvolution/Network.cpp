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
            nodes[i].enabled = true;
        }
        
        for(size_t i = 0; i < output_size; ++i) {
            nodes[input_size + i].enabled = true;
        }
    }
    
    bool Network::outputs_off() const {
        size_t size = nodes.size();
        
        for(size_t i = input_size; i < size; ++i) {
            if(nodes[i].acts == 0)
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
                if(nodes[i].computed && nodes[i].enabled) {
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
    
    void Network::mutate_weights() {
        for(Link* link : links) {
            if(random() < weights_reset_prob)
                link->weight = gaussian_random();
            
            else if(random() < weights_mutate_prob)
                link->weight += weights_mutation_power * random() * randposneg();
        }
        
        Node& node = nodes[input_size - 1];
        
        if(random() < weights_reset_prob)
            node.value = gaussian_random();
        
        else if(random() < weights_mutate_prob)
            node.value += weights_mutation_power * random() * randposneg();
    }
    
    void Network::mutate_topology_add_node(innov_map *map, size_t *innov) {
        size_t ls = links.size();
        
        if(ls != 0) {
            for(size_t n = 0; n < timeout; ++n) {
                Link* link = links[rand64() % ls];
                size_t node = create_node();
                
                if(!link->enabled) continue;
                
                link->enabled = false;
                
                {
                    Link* link1 = new Link();
                    link1->i = link->i;
                    link1->j = node;
                    
                    set_innov(map, innov, link1);
                    
                    link1->weight = 1.0f;
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
                
                break;
            }
        }
    }
    
    void Network::mutate_topology_add_link(innov_map *map, size_t *innov) {
        size_t size = nodes.size();
        
        for(size_t n = 0; n < timeout; ++n) {
            size_t i = rand64() % size;
            size_t j = input_size + (rand64() % (size - input_size));
            
            Link l;
            l.i = i;
            l.j = j;
            
            auto it = set.find(&l);
            if(it != set.end()) {
                continue;
            }else{
                Link* link = new Link(l);
                
                set_innov(map, innov, link);
                
                link->weight = gaussian_random();
                link->enabled = true;
                
                insert(link);
            }
            
            break;
        }
    }
    
    void Network::mutate_toggle_link_enable(size_t times) {
        size_t ls = links.size();
        
        if(ls != 0) {
            for(size_t n = 0; n < times; ++n) {
                Link* link = links[rand64() % ls];
                link->enabled = !link->enabled;
            }
        }
    }
    
    void Network::mutate_toggle_node_enable(size_t times) {
        size_t ns = nodes.size() - input_size - output_size;
        
        if(ns != 0) {
            for(size_t n = 0; n < times; ++n) {
                Node& node = nodes[rand64() % ns];
                node.enabled = !node.enabled;
            }
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
        
        for(size_t i = 0; i < size; ++i) {
            if(i < as && i < bs) {
                if(A->nodes[i].enabled && B->nodes[i].enabled) {
                    C->nodes[i].enabled = true;
                }else if(A->nodes[i].enabled) {
                    C->nodes[i].enabled = random() < disable_inheritance ? false : true;
                }else if(B->nodes[i].enabled) {
                    C->nodes[i].enabled = random() < disable_inheritance ? false : true;
                }else{
                    C->nodes[i].enabled = false;
                }
            }else if(i < as) {
                C->nodes[i].enabled = A->nodes[i].enabled;
            }else{
                C->nodes[i].enabled = B->nodes[i].enabled;
            }
        }
        
        bool avg = random() < mate_avg_prob;
        
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
                    link.enabled = random() < disable_inheritance ? false : true;
                }else if((*itB)->enabled) {
                    link.enabled = random() < disable_inheritance ? false : true;
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
            }else{
                (*it)->weight = ((*it)->weight + link.weight) * 0.5f;
            }
        }
    }
    
    float Network::distance(const Network *A, const Network *B) {
        std::vector<Link*>::const_iterator itA, itB;
        
        itA = A->links.begin();
        itB = B->links.begin();
        
        size_t miss = 0;
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
                W += fabsf((*itA)->weight - (*itB)->weight);
                
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
        
        return (miss + weights_power * W) / (float) std::max(A->links.size(), B->links.size());
    }
    
}
