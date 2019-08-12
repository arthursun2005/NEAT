//
//  network.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ne_network_h
#define ne_network_h

#include "ne.h"
#include <vector>
#include <unordered_set>

struct ne_species;

class ne_network
{
    
public:
    
    ne_network() {}
    
    ne_network(const ne_network& network);
    
    ne_network& operator = (const ne_network& network);
    
    ~ne_network() {
        clear();
    }
    
    inline ne_node* inputs() {
        return nodes.data();
    }
    
    inline ne_node* outputs() {
        return nodes.data() + input_size;
    }
    
    inline size_t size() const {
        return nodes.size();
    }
    
    inline size_t complexity() const {
        return links.size();
    }
    
    void reset(size_t inputs, size_t outputs);
    
    void initialize(innov_map* map, size_t* innov);
    
    void compute();
    
    void clear();
    
    void mutate_weights(const ne_params& params);
    
    void mutate_topology_add_node(innov_map* map, size_t* innov, const ne_params& params);
    
    void mutate_topology_add_link(innov_map* map, size_t* innov, const ne_params& params);
    
    void mutate_toggle_link_enable(size_t times);
    
    float fitness;
    
    float adjusted_fitness;
    
    float rank;
    
    bool killed;
    
    static void crossover(const ne_network* A, const ne_network* B, ne_network* C, const ne_params& params);
    
    static float distance(const ne_network* A, const ne_network* B, const ne_params& params);
    
    size_t age;
    
    ne_species* sp;
    
protected:
    
    bool outputs_off() const;
    
    void insert(ne_link* link);
    
    size_t create_node();
    
    size_t input_size;
    size_t output_size;
    
    void resize();
    
    std::vector<ne_node> nodes;
    std::vector<ne_link*> links;
    
    std::unordered_set<ne_link*> set;
    
};

inline bool ne_network_sort_adjusted_fitness (const ne_network* A, const ne_network* B) {
    return A->adjusted_fitness < B->adjusted_fitness;
}

inline bool ne_network_sort_fitness (const ne_network* A, const ne_network* B) {
    return A->fitness < B->fitness;
}

inline bool ne_network_sort_rank (const ne_network* A, const ne_network* B) {
    return A->rank < B->rank;
}

inline void ne_mutate_network(ne_network* network, const ne_params& params, innov_map* map, size_t* innov) {
    if(ne_random() < params.mutate_weights_prob) {
        network->mutate_weights(params);
    }
    
    if(ne_random() < params.new_node_prob) {
        network->mutate_topology_add_node(map, innov, params);
    }
    
    if(ne_random() < params.new_link_prob) {
        network->mutate_topology_add_link(map, innov, params);
    }
    
    if(ne_random() < params.toggle_link_enable_prob) {
        network->mutate_toggle_link_enable(1);
    }
}
    

#endif /* ne_network_h */
