//
//  Network.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef Network_h
#define Network_h

#include "Node.h"
#include <vector>

namespace NE {

    class Network
    {
        
    public:
        
        Network() {}
        
        Network(const Network& network);
        
        Network& operator = (const Network& network);
        
        ~Network() {
            clear();
        }
        
        inline Node* inputs() {
            return nodes.data();
        }
        
        inline Node* outputs() {
            return nodes.data() + input_size;
        }
        
        inline size_t size() const {
            return nodes.size();
        }
        
        inline size_t complexity() const {
            return links.size();
        }
        
        void reset(size_t inputs, size_t outputs);
        
        void compute();
        
        void clear();
        
        void mutate(innov_set* set, size_t* innov);

        float fitness;
        
        float strength;
        
        bool killed;
        
        size_t age;
        
        size_t k;
                
        static void crossover(Network* A, Network* B, Network* C);
        
        static float distance_topology(Network* A, Network* B);
        
        static float distance_weights(Network* A, Network* B);
        
    protected:
        
        bool outputs_off() const;
        
        void insert(Link* link);
        
        void disable(Link* link);
        
        size_t create_node();
                
        size_t input_size;
        size_t output_size;
        
        void resize();
        
        std::vector<Node> nodes;
        std::vector<Link*> links;
        
        innov_set set;
        
    };
    
    inline bool network_sort (Network* A, Network* B) {
        return A->strength < B->strength;
    }
    
}

#endif /* Network_h */
