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
#include <unordered_set>

namespace NE {
    
    struct Species;

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
        
        void mutate_weights();
                
        void mutate_topology(innov_map* map, size_t* innov);

        float fitness;
        
        float placement;
                                
        static void crossover(const Network* A, const Network* B, Network* C);
        
        static float distance(const Network* A, const Network* B);
        
        size_t age;
        
        Species* sp;
        
    protected:
        
        bool outputs_off() const;
        
        void insert(Link* link);
                
        size_t create_node();
                
        size_t input_size;
        size_t output_size;
        
        void resize();
        
        std::vector<Node> nodes;
        std::vector<Link*> links;
        
        std::unordered_set<Link*> set;
        
    };
    
    inline bool network_sort (const Network* A, const Network* B) {
        return A->placement > B->placement;
    }
    
}

#endif /* Network_h */
