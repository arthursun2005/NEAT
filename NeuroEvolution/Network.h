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
#include <unordered_map>

namespace NE {

    struct Innov
    {
        size_t i;
        size_t j;
        
        Innov() {}
        
        Innov(size_t i, size_t j) : i(i), j(j) {}
        
        Innov(const Link& l) : i(l.i), j(l.j) {}
    };
    
    typedef std::unordered_map<Innov, size_t> innov_map;
    
}

template <>
struct std::hash<NE::Innov>
{
    inline size_t operator () (const NE::Innov& x) const {
        return x.i ^ x.j;
    }
};

template <>
struct std::equal_to<NE::Innov>
{
    inline bool operator () (const NE::Innov& a, const NE::Innov& b) const {
        return a.i == b.i && a.j == b.j;
    }
};

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
            return links.size();
        }
        
        void reset(size_t inputs, size_t outputs);
        
        void compute();
        
        void mutate(innov_map* map, size_t* innov);
        
        float fitness;
        
        size_t age;

    protected:
                
        void insert(Link* link);
        
        void remove(std::list<Link*>::iterator link);
        
        size_t create_node();
        
        std::list<Link*>::iterator random_link();
        
        bool has_node(size_t i, size_t j) const;
        
        size_t input_size;
        size_t output_size;
        
        void resize();
        
        std::vector<Node> nodes;
        
        std::list<Link*> links;
        
        size_t next;

    };
    
    inline bool network_sort (Network* A, Network* B) {
        return A->fitness < B->fitness;
    }
    
}

#endif /* Network_h */
