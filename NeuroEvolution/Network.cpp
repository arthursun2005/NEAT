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
        node->value = node->function(node->value + node->bias);
    }
    
    inline void insert_link(Link* A, Link* B) {
        if(A->prev != nullptr) {
            A->prev->next = B;
        }
        
        B->next = A;
        B->prev = A->prev;
        A->prev = B;
    }
    
    void Network::reset(size_t inputs, size_t outputs) {
        nodes.resize(inputs + outputs);
        clear();
        
        for(Node& node : nodes) {
            node.bias = gaussian_randomf();
            node.function.randomlize();
            node.links = 0;
            node.begin = nullptr;
        }
        
        input_size = inputs;
        output_size = outputs;
        
        next = -1;
    }
    
    void Network::clear() {
        while(links != nullptr) {
            Link* link = links;
            links = links->next;
            delete link;
        }
        
        links_size = 0;
    }
    
    void Network::insert(Link *link) {
        Link** A = &(nodes[link->j].begin);
        
        if(links == nullptr) {
            links = link;
            links->prev = nullptr;
            links->next = nullptr;
        }else{
            if((*A) == nullptr) {
                insert_link(links, link);
                links = link;
            }else{
                insert_link(*A, link);
                
                if(*A == links)
                    links = link;
            }
        }
        
        *A = link;
        
        ++nodes[link->i].links;
        ++nodes[link->j].links;
        
        ++links_size;
    }
    
    void Network::remove(Link *link) {
        if(link->prev == nullptr) {
            links = link->next;
            if(links != nullptr)
                links->prev = nullptr;
        }else{
            link->prev->next = link->next;
            
            if(link->next != nullptr)
                link->next->prev = link->prev;
        }
        
        if(nodes[link->j].begin == link) {
            if(link->next != nullptr && link->next->j == link->j) {
                nodes[link->j].begin = link->next;
            }else{
                nodes[link->j].begin = nullptr;
            }
        }
        
        --nodes[link->i].links;
        --nodes[link->j].links;
        
        size_t io = input_size + output_size;
        
        if(nodes[link->i].links == 0 && link->i >= io) {
            next = link->i < next ? link->i : next;
        }
        
        if(nodes[link->j].links == 0 && link->j >= io) {
            next = link->j < next ? link->j : next;
        }
        
        --links_size;
    }
    
    void Network::compute() {
        size_t size = nodes.size();
        
        for(size_t i = 0; i < input_size; ++i) {
            compute_node(nodes.data() + i);
        }
        
        for(size_t i = input_size; i < size; ++i) {
            nodes[i].value = float_t(0);
        }
        
        Link* link = links;
        while(link != nullptr) {
            nodes[link->j].value += nodes[link->i].value * link->weight;
            
            if(link->next == nullptr || link->next->j != link->j) {
                compute_node(nodes.data() + link->j);
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
            
            Link* link = nodes[n].begin;
            while(link != nullptr && link->j == i) {
                if(link->i == j)
                    return true;
                
                stack.push(link->i);
                
                link = link->next;
            }
            
        }
        
        return false;
    }
    
    Link* Network::random_link() const {
        size_t n = rand64() % links_size;
        Link* link = links;
        while(n-- > 0) link = link->next;
        return link;
    }
    
    size_t Network::create_node() {
        Node node;
        node.begin = nullptr;
        node.bias = gaussian_randomf();
        node.links = 0;
        
        size_t i = nodes.size();
        /*
        if(next < i) {
            while(next != i && nodes[next].links != 0) {
                ++next;
            }
            
            if(next != i) {
                nodes[next] = node;
                return next;
            }
        }
        */
        nodes.push_back(node);
        
        return i;
    }
    
    void Network::mutate(size_t innovation) {
        uint32_t k = rand32() & 0xffff;
        
        if(k & 1) {
            if(k <= 0x7fff && links_size != 0) {
                Link* link = random_link();
                size_t node = create_node();
                
                Link* new_link = new Link();
                new_link->i = link->i;
                new_link->j = node;
                new_link->innovation = innovation;
                new_link->weight = gaussian_randomf();
                
                remove(link);
                
                link->i = node;
                
                insert(link);
                insert(new_link);
            }else{
                size_t size = nodes.size();
                
                if(rand32() & 1 || links_size == 0) {
                    size_t i = rand64() % size;
                    size_t j = input_size + (rand64() % (size - input_size));
                    
                    Link* link = nodes[j].begin;
                    while(link != nullptr && link->j == j) {
                        if(link->i == i)
                            return;
                        
                        link = link->next;
                    }
                    
                    if(!has_node(i, j) && i != j) {
                        Link* link = new Link();
                        link->i = i;
                        link->j = j;
                        link->innovation = innovation;
                        link->weight = gaussian_randomf();
                        insert(link);
                    }
                }else{
                    Link* link = random_link();
                    remove(link);
                    delete link;
                }
            }
        }else{
            size_t i = rand64() % nodes.size();
            nodes[i].function.randomlize();
            Link* link = nodes[i].begin;
            
            if(nodes[i].links != 0) {
                while(link != nullptr && link->j == i) {
                    link->weight = gaussian_randomf();
                    link = link->next;
                }
            }
        }
        
    }
    
    Network::Network(const Network& network) : links(nullptr) {
        reset(network.input_size, network.output_size);
        *this = network;
    }
    
    Network& Network::operator = (const Network &network) {
        clear();
        
        nodes = network.nodes;
        next = network.next;
        
        input_size = network.input_size;
        output_size = network.output_size;
        
        fitness = network.fitness;
        
        size_t size = nodes.size();
        
        for(size_t i = 0; i < size; ++i) {
            nodes[i].links = 0;
            nodes[i].begin = nullptr;
        }
        
        for(size_t i = 0; i < size; ++i) {
            if(network.nodes[i].begin != nullptr) {
                Link* link = new Link();
                *link = *network.nodes[i].begin;
                insert(link);
            }
        }
        
        Link* link = network.links;
        
        while(link != nullptr) {
            if(network.nodes[link->j].begin != link) {
                insert(new Link(*link));
            }
            link = link->next;
        }
        
        return *this;
    }
    
}
