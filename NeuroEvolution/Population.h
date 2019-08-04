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
            ++innovation;
            
            size_t size = networks.size();
            size_t half_size = size / 2;
            
            std::sort(networks.data(), networks.data() + size, network_sort);
            
            for(size_t i = 0; i < half_size; ++i) {
                *(networks[i]) = *(networks[size - i - 1]);
                networks[i]->mutate(innovation);
                networks[i]->age = 0;
            }
            
            for(size_t i = 0; i < size; ++i) {
                //++networks[i]->age;
            }

            return networks.back();
        }
        
    protected:
        
        size_t innovation;
        std::vector<Network*> networks;
        
    };
    
}

#endif /* Population_h */
