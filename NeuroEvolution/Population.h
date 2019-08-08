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
            
            for(size_t i = 0; i < size; ++i) {
                networks[i]->k = 0;
                networks[i]->strength = 0.0f;
                
                for(size_t j = 0; j < size; ++j) {
                    float tq = Network::distance_topology(networks[i], networks[j]);
                    float wq = Network::distance_weights(networks[i], networks[j]);
                    float ts = tq + wq;
                    if(ts < similar_rate) {
                        ++networks[i]->k;
                        networks[i]->strength += networks[j]->fitness;
                    }
                }
                
                networks[i]->strength = networks[i]->strength / (float) (networks[i]->k * networks[i]->k);
            }
            
            set.clear();
            
            std::sort(networks.data(), networks.data() + size, network_sort);
            
            /*
            std::vector<Network*> ns;
            
            set.clear();
            
            for(size_t i = 0; i < size; ++i) {
                networks[i]->k = 0;
                networks[i]->strength = 0.0f;
                
                for(size_t j = 0; j < size; ++j) {
                    float tq = Network::distance_topology(networks[i], networks[j]);
                    float wq = Network::distance_weights(networks[i], networks[j]);
                    float ts = tq + wq;
                    if(ts < similar_rate) {
                        ++networks[i]->k;
                        networks[i]->strength += networks[j]->fitness;
                    }
                }
                networks[i]->strength = networks[i]->fitness / (float) (networks[i]->k);
            }
            
            std::sort(networks.data(), networks.data() + size, network_sort);
            
            size_t kills = 0;
            for(size_t i = 0; i < size; ++i) {
                if(networks[i]->age >= minimum_age && kills != half_size) {
                    networks[i]->killed = true;
                    ++kills;
                }else{
                    networks[i]->killed = false;
                    ns.push_back(networks[i]);
                }
            }
            
            {
                size_t i = size;
                while(i-- > 0) {
                    if(networks[i]->killed) {
                        delete networks[i];
                        networks.erase(networks.begin() + i);
                    }
                }
            }
            
            size_t babies = kills;
            size_t w = ns.size();
            
            for(size_t i = 0; i < babies; ++i) {
                Network* baby = new Network();
                size_t i1 = random() * w;
                size_t i2 = random() * w;
                size_t i3 = random() * w;
                size_t i4 = random() * w;
                
                Network::crossover(ns[i1 > i2 ? i1 : i2], ns[i3 > i4 ? i3 : i4], baby);
                
                if(rand32() & 1) {
                    baby->mutate(&set, &innovation);
                }
                
                networks.push_back(baby);
            }
            */
            
            for(size_t i = 0; i < half_size; ++i) {
                size_t i1 = size - 1 - size_t(random() * half_size);
                size_t i2 = size - 1 - size_t(random() * half_size);
                
                Network::crossover(networks[i1], networks[i2], networks[i]);
                networks[i]->mutate(&set, &innovation);
            }

            
            size_t idx = 0;
            
            for(size_t i = 0; i < size; ++i) {
                ++networks[i]->age;
                
                if(networks[i]->fitness > networks[idx]->fitness)
                    idx = i;
            }
            
            return networks[idx];
             
        }
        
        size_t innovation;
        innov_set set;
        
    protected:
        
        std::vector<Network*> networks;
        std::vector<Species> species;
        
    };
    
}

#endif /* Population_h */
