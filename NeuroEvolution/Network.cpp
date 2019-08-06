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
    
    inline void compute_node(Node* node) {
        node->value = node->function(node->value);
    }
    
    void Network::reset(size_t inputs, size_t outputs) {
        nodes.resize(inputs + outputs);
        
        for(Link* link : links)
            delete link;
        
        links.clear();
        
        for(Node& node : nodes) {
            node.function.randomlize();
            node.links = 0;
            node.begin = links.end();
        }
        
        input_size = inputs;
        output_size = outputs;
        
        next = -1;
        age = 0;
    }
    
    void Network::insert(Link* link) {
        std::list<Link*>::iterator* A = &(nodes[link->j].begin);
        
        links.insert(*A, link);
        
        ++nodes[link->i].links;
        ++nodes[link->j].links;
    }
    
    void Network::remove(std::list<Link*>::iterator link) {
        std::list<Link*>::iterator q = link;
        ++q;
        if(nodes[(*link)->j].begin == link) {
            if(q != links.end() && (*q)->j == (*link)->j) {
                nodes[(*link)->j].begin = q;
            }else{
                nodes[(*link)->j].begin = links.end();
            }
        }
        
        --nodes[(*link)->i].links;
        --nodes[(*link)->j].links;
        
        size_t io = input_size + output_size;
        
        if(nodes[(*link)->i].links == 0 && (*link)->i >= io) {
            next = (*link)->i < next ? (*link)->i : next;
        }
        
        if(nodes[(*link)->j].links == 0 && (*link)->j >= io) {
            next = (*link)->j < next ? (*link)->j : next;
        }
        
        links.erase(link);
    }
    
    void Network::compute() {
        size_t size = nodes.size();
        
        for(size_t i = 0; i < input_size; ++i) {
            compute_node(nodes.data() + i);
        }
        
        for(size_t i = input_size; i < size; ++i) {
            nodes[i].value = float_t(0);
        }
        
        std::list<Link*>::iterator link = links.begin();
        Node* ns = nodes.data();
        while(link != links.end()) {
            nodes[(*link)->j].value += nodes[(*link)->i].value * (*link)->weight;
            
            std::list<Link*>::iterator q = link;
            ++q;
            
            if(q == links.end() || (*q)->j != (*link)->j) {
                compute_node(ns + (*link)->j);
            }

            ++link;
        }
    }
    
    bool Network::has_node(size_t i, size_t j) const {
        std::stack<size_t> stack;
        
        stack.push(i);
        
        while(!stack.empty()) {
            size_t n = stack.top();
            stack.pop();
            
            std::list<Link*>::iterator link = nodes[n].begin;
            while(link != links.end() && (*link)->j == i) {
                if((*link)->i == j)
                    return true;
                
                stack.push((*link)->i);
                
                ++link;
            }
            
        }
        
        return false;
    }
    
    std::list<Link*>::iterator Network::random_link() {
        size_t n = rand64() % links.size();
        std::list<Link*>::iterator link = links.begin();
        while(n-- > 0) ++link;
        return link;
    }
    
    size_t Network::create_node() {
        Node node;
        node.begin = links.end();
        node.links = 0;
        
        size_t i = nodes.size();
        
        if(next < i) {
            while(next != i && nodes[next].links != 0) {
                ++next;
            }
            
            if(next != i) {
                nodes[next] = node;
                return next;
            }
        }
        
        nodes.push_back(node);
        
        return i;
    }
    
    void Network::mutate(innov_map *map, size_t *innov) {
        size_t size = nodes.size();
        
        {
            size_t q = 1 + links.size();
            
            std::list<Link*>::iterator link = links.begin();
            while(link != links.end()) {
                if((rand64() % q) == 0) (*link)->weight += random() * randposneg();
                ++link;
            }
        }
        
        uint32_t k = rand32() & 0xffff;
        
        size_t ls = links.size();
        
        if(k & 1) {
            if(k <= 0x7fff && ls != 0) {
                std::list<Link*>::iterator link = random_link();
                size_t node = create_node();
                
                Link new_link;
                new_link.i = (*link)->i;
                new_link.j = node;
                
                auto it = map->find(Innov(new_link));
                
                if(it != map->end()) {
                    new_link.innovation = it->second;
                }else{
                    new_link.innovation = *innov;
                    (*innov)++;
                }
                
                new_link.weight = gaussian_random();
                
                remove(link);
                
                (*link)->i = node;
                
                insert(*link);
                insert(new Link(new_link));
            }else{
                if(rand32() & 1 || ls == 0) {
                    size_t i = rand64() % size;
                    size_t j = input_size + (rand64() % (size - input_size));
                    
                    std::list<Link*>::iterator link = nodes[j].begin;
                    while(link != links.end() && (*link)->j == j) {
                        if((*link)->i == i)
                            return;
                        
                        ++link;
                    }
                    
                    if(!has_node(i, j) && i != j) {
                        Link link;
                        link.i = i;
                        link.j = j;
                        
                        auto it = map->find(Innov(link));
                        
                        if(it != map->end()) {
                            link.innovation = it->second;
                        }else{
                            link.innovation = *innov;
                            (*innov)++;
                        }
                        
                        link.weight = gaussian_random();
                        insert(new Link(link));
                    }
                }else{
                    std::list<Link*>::iterator link = random_link();
                    remove(link);
                    delete *link;
                }
            }
        }else{
            size_t i = rand64() % nodes.size();
            nodes[i].function.randomlize();
            std::list<Link*>::iterator link = nodes[i].begin;
            
            if(nodes[i].links != 0) {
                while(link != links.end() && (*link)->j == i) {
                    (*link)->weight = gaussian_random();
                    ++link;
                }
            }
        }
    }
    
    Network::Network(const Network& network) {
        reset(network.input_size, network.output_size);
        *this = network;
    }
    
    Network& Network::operator = (const Network &network) {
        for(Link* link : links)
            delete link;
        
        links.clear();
        
        nodes = network.nodes;
        next = network.next;
        
        input_size = network.input_size;
        output_size = network.output_size;
        
        fitness = network.fitness;
        
        age = network.age;
        
        size_t size = nodes.size();
        
        for(size_t i = 0; i < size; ++i) {
            nodes[i].links = 0;
            nodes[i].begin = links.end();
        }
        
        for(size_t i = 0; i < size; ++i) {
            if(network.nodes[i].begin != network.links.end()) {
                insert(new Link(**network.nodes[i].begin));
            }
        }
        
        std::list<Link*>::const_iterator link = network.links.begin();
        
        while(link != network.links.end()) {
            if(network.nodes[(*link)->j].begin != link) {
                insert(new Link(**link));
            }
            ++link;
        }
        
        return *this;
    }
    
}
