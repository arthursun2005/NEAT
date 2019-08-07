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
        const Link* link;
        
        Innov() {}
        
        Innov(const Link* link) : link(link) {}
    };
    
    typedef std::unordered_map<Innov, size_t> innov_map;
    
}

template <>
struct std::hash<NE::Innov>
{
    inline size_t operator () (const NE::Innov& x) const {
        return (x.link->i ^ x.link->j) + (x.link->i << 17) + 71 * x.link->j;
    }
};

template <>
struct std::equal_to<NE::Innov>
{
    inline bool operator () (const NE::Innov& a, const NE::Innov& b) const {
        return a.link->i == b.link->i && a.link->j == b.link->j;
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
        
        void clear();
        
        void mutate(innov_map* map, size_t* innov);
        
        float fitness;
        
        size_t age;
        
        size_t k;
        
        static void crossover(Network* A, Network* B, Network* C);
        
        static float closeness(Network* A, Network* B, float q);

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
        std::list<Link*> innovs;
        
        size_t next;

    };
    
    inline bool network_sort (Network* A, Network* B) {
        //return A->fitness / (float) A->k < B->fitness / (float) B->k;
        return A->fitness < B->fitness;
    }
    
}

#endif /* Network_h */
