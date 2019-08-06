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
        links.clear();
        
        for(Node& node : nodes) {
            node.function.randomlize();
            node.links = 0;
            node.begin = nullptr;
        }
        
        input_size = inputs;
        output_size = outputs;
        
        next = -1;
        age = 0;
    }
    
    void Network::insert(list<Link>::type* link) {
        list<Link>::type** A = &(nodes[link->data.j].begin);
        
        *A = links.insert(*A, link);
        
        ++nodes[link->data.i].links;
        ++nodes[link->data.j].links;
    }
    
    void Network::remove(list<Link>::type *ptr) {
        if(nodes[ptr->data.j].begin == ptr) {
            if(ptr->next != nullptr && ptr->next->data.j == ptr->data.j) {
                nodes[ptr->data.j].begin = ptr->next;
            }else{
                nodes[ptr->data.j].begin = nullptr;
            }
        }
        
        links.remove(ptr);
        
        --nodes[ptr->data.i].links;
        --nodes[ptr->data.j].links;
        
        size_t io = input_size + output_size;
        
        if(nodes[ptr->data.i].links == 0 && ptr->data.i >= io) {
            next = ptr->data.i < next ? ptr->data.i : next;
        }
        
        if(nodes[ptr->data.j].links == 0 && ptr->data.j >= io) {
            next = ptr->data.j < next ? ptr->data.j : next;
        }
    }
    
    void Network::compute() {
        size_t size = nodes.size();
        
        for(size_t i = 0; i < input_size; ++i) {
            compute_node(nodes.data() + i);
        }
        
        for(size_t i = input_size; i < size; ++i) {
            nodes[i].value = float_t(0);
        }
        
        list<Link>::type* link = links.begin;
        Node* ns = nodes.data();
        while(link != nullptr) {
            nodes[link->data.j].value += nodes[link->data.i].value * link->data.weight;
            
            if(link->next == nullptr || link->next->data.j != link->data.j) {
                compute_node(ns + link->data.j);
            }

            link = link->next;
        }
    }
    
    bool Network::has_node(size_t i, size_t j) const {
        std::stack<size_t> stack;
        
        stack.push(i);
        
        while(!stack.empty()) {
            size_t n = stack.top();
            stack.pop();
            
            list<Link>::type* link = nodes[n].begin;
            while(link != nullptr && link->data.j == i) {
                if(link->data.i == j)
                    return true;
                
                stack.push(link->data.i);
                
                link = link->next;
            }
            
        }
        
        return false;
    }
    
    list<Link>::type* Network::random_link() const {
        size_t n = rand64() % links.size;
        list<Link>::type* link = links.begin;
        while(n-- > 0) link = link->next;
        return link;
    }
    
    size_t Network::create_node() {
        Node node;
        node.begin = nullptr;
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
            size_t q = 1 + links.size;
            
            list<Link>::type* link = links.begin;
            while(link != nullptr) {
                if((rand64() % q) == 0) link->data.weight += random() * randposneg();
                link = link->next;
            }
        }
        
        uint32_t k = rand32() & 0xffff;
        
        if(k & 1) {
            if(k <= 0x7fff && links.size != 0) {
                list<Link>::type* link = random_link();
                size_t node = create_node();
                
                Link new_link;
                new_link.i = link->data.i;
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
                
                link->data.i = node;
                
                insert(link);
                insert(new list<Link>::type(new_link));
            }else{
                if(rand32() & 1 || links.size == 0) {
                    size_t i = rand64() % size;
                    size_t j = input_size + (rand64() % (size - input_size));
                    
                    list<Link>::type* link = nodes[j].begin;
                    while(link != nullptr && link->data.j == j) {
                        if(link->data.i == i)
                            return;
                        
                        link = link->next;
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
                        insert(new list<Link>::type(link));
                    }
                }else{
                    list<Link>::type* link = random_link();
                    remove(link);
                    delete link;
                }
            }
        }else{
            size_t i = rand64() % nodes.size();
            nodes[i].function.randomlize();
            list<Link>::type* link = nodes[i].begin;
            
            if(nodes[i].links != 0) {
                while(link != nullptr && link->data.j == i) {
                    link->data.weight = gaussian_random();
                    link = link->next;
                }
            }
        }
    }
    
    Network::Network(const Network& network) {
        reset(network.input_size, network.output_size);
        *this = network;
    }
    
    Network& Network::operator = (const Network &network) {
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
            nodes[i].begin = nullptr;
        }
        
        for(size_t i = 0; i < size; ++i) {
            if(network.nodes[i].begin != nullptr) {
                insert(new list<Link>::type(network.nodes[i].begin->data));
            }
        }
        
        list<Link>::type* link = network.links.begin;
        
        while(link != nullptr) {
            if(network.nodes[link->data.j].begin != link) {
                insert(new list<Link>::type(link->data));
            }
            link = link->next;
        }
        
        return *this;
    }
    
}
