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
            return links.size;
        }
        
        void reset(size_t inputs, size_t outputs);
        
        void compute();
        
        void mutate(size_t);
        
        float fitness;
        
        size_t age;

    protected:
                
        void insert(list<Link>::type* link);
        
        void remove(list<Link>::type* link);
        
        size_t create_node();
        
        list<Link>::type* random_link() const;
        
        bool has_node(size_t i, size_t j) const;
        
        size_t input_size;
        size_t output_size;
        
        void resize();
        
        std::vector<Node> nodes;
        list<Link> links;
        
        size_t next;

    };
    
    inline bool network_sort (Network* A, Network* B) {
        return A->fitness < B->fitness;
    }
    
}

#endif /* Network_h */
