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
        
        void speciate() {
            species.clear();
            
            size_t size = networks.size();
            for(size_t i = 0; i < size; ++i) {
                Species* sp = nullptr;
                float mc = similar_rate;
                
                for(Species& s : species) {
                    Network* j = s.networks.front();
                    float ts = Network::distance(networks[i], j);
                    if(ts < mc) {
                        mc = ts;
                        sp = &s;
                    }
                }
                
                if(sp == nullptr) {
                    Species s;
                    s.networks.push_back(networks[i]);
                    species.push_back(s);
                }else{
                    sp->networks.push_back(networks[i]);
                }
            }
            
            size_t offsprings = 0;
            size_t allowed_offsprings = size / 2;
            float total_fitness = 0.0f;
            
            for(Species& s : species) {
                float tf = 0.0f;
                
                for(Network* n : s.networks) {
                    tf += n->fitness;
                }
                
                s.avg_fitness = tf / (float) (s.networks.size());
                
                total_fitness += s.avg_fitness;
            }
            
            std::sort(species.data(), species.data() + species.size(), species_sort);
            
            for(Species& s : species) {
                s.offsprings = roundf((allowed_offsprings * s.avg_fitness) / total_fitness);
                offsprings += s.offsprings;
            }
            
            species.back().offsprings += allowed_offsprings > offsprings ? allowed_offsprings - offsprings : 0;
        }
        
        Network* reproduce() {
            map.clear();
            
            size_t size = networks.size();
            
            std::vector<Network> ns;
            
            for(Species& sp : species) {
                size_t spsize = sp.networks.size();
                for(size_t n = 0; n < sp.offsprings; ++n) {
                    Network baby;
                    
                    size_t i1 = random() * spsize;
                    size_t i2 = random() * spsize;
                    
                    Network::crossover(sp.networks[i1], sp.networks[i2], &baby);
                    
                    if(rand32() & 1) baby.mutate(&map, &innovation);
                    
                    ns.push_back(baby);
                }
            }
            
            std::sort(networks.data(), networks.data() + size, network_sort);
            size_t q = ns.size();
            
            for(size_t i = 0; i < q; ++i) {
                *(networks[i]) = ns[i];
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
        innov_map map;
        
        std::vector<Network*> networks;
        std::vector<Species> species;
        
    };
    
}

#endif /* Population_h */
