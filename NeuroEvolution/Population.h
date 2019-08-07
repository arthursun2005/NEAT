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
            for(Network* network : networks) {
                network->reset(inputs, outputs);
            }
            
            innovation = 0;
        }
        
        Network* operator [] (size_t i) {
            return networks[i];
        }
        
        Network* step() {
            size_t size = networks.size();
            size_t half_size = size / 2;
            
            std::sort(networks.data(), networks.data() + size, network_sort);
            
            set.clear();
            
            for(size_t i = 0; i < half_size; ++i) {
                size_t i1 = size - 1 - size_t(random() * half_size);
                size_t i2 = size - 1 - size_t(random() * half_size);
                
                Network::crossover(networks[i1], networks[i2], networks[i]);
                
                if(rand32() & 1) {
                    networks[i]->mutate(&set, &innovation);
                }
            }
            
            for(size_t i = 0; i < size; ++i) {
                ++networks[i]->age;
            }
            
            return networks.back();
        }
        
        size_t innovation;
        innov_set set;
        
    protected:
        
        std::vector<Network*> networks;
        std::vector<Species> species;
        
    };
    
}

#endif /* Population_h */
