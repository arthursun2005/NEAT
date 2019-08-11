//
//  Population.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/31/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef Population_h
#define Population_h

#include "Network.h"
#include "Species.h"

namespace NE {
    
    class Population
    {
        
    public:
        
        ~Population() {
            for(Network* network : networks) {
                delete network;
            }
            
            for(Species* sp : species) {
                delete sp;
            }
        }
        
        inline void resize(size_t population) {
            for(Network* network : networks) {
                delete network;
            }
            
            networks.resize(population);
            
            for(Network*& network : networks) {
                network = new Network();
            }
        }
        
        inline void reset(size_t inputs, size_t outputs) {
            map.clear();
            innovation = 0;
            
            for(Network* network : networks) {
                network->reset(inputs, outputs);
                
                while(network->complexity() == 0)
                    network->mutate_topology_add_link(&map, &innovation);
            }

            _speciate();
        }
        
        inline Network* operator [] (size_t i) {
            return networks[i];
        }
        
        Network* select();
        
        void reproduce();
        
        size_t innovation;
        
        std::vector<Network*> networks;
        std::vector<Species*> species;
        
        float best_fitness;
        
    private:
        
        innov_map map;
        
        void _add(Network* n);
        
        void _remove(Network* n);
        
        inline void _speciate() {
            species.clear();
            
            for(Network* n : networks) {
                _add(n);
            }
        }
        
    };
    
}

#endif /* Population_h */
