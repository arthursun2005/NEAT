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
                network->mutate_topology(&map, &innovation);
            }
            
            _speciate();
        }
        
        Network* operator [] (size_t i) {
            return networks[i];
        }
        
        void select() {
            size_t size = networks.size();
            
            size_t offsprings = 0;
            size_t allowed_offsprings = roundf(size * (1.0f - survival_thresh));
            float total_fitness = 0.0f;
            
            best_fitness = -FLT_MAX;
            
            for(Species* sp : species) {
                sp->avg_fitness = 0.0f;
                sp->best_fitness = -FLT_MAX;
                
                for(Network* n : sp->networks) {
                    if(n->fitness < 0.0f) n->placement = 0.0f;
                    else n->placement = n->fitness;
                    
                    sp->best_fitness = std::max(sp->best_fitness, n->placement);
                    
                    n->placement /= (float) sp->networks.size();
                    
                    sp->avg_fitness += n->placement;
                }

                if(sp->best_fitness > sp->max_fitness) {
                    sp->max_fitness = sp->best_fitness;
                    sp->time_since_improvement = 0;
                }else{
                    ++sp->time_since_improvement;
                }
                
                if(sp->best_fitness > best_fitness) {
                    best_fitness = sp->best_fitness;
                }
            }
            
            for(Species* sp : species) {
                if(sp->time_since_improvement >= dead_species_rate && sp->best_fitness < best_fitness) {
                    for(Network* n : sp->networks) {
                        n->placement = 0.0f;
                        sp->avg_fitness = 0.0f;
                        sp->time_since_improvement = 0;
                    }
                }
                
                total_fitness += sp->avg_fitness;
            }
            
            std::sort(species.data(), species.data() + species.size(), species_sort);
            
            for(Species* s : species) {
                s->offsprings = floorf(allowed_offsprings * (s->avg_fitness / total_fitness));
                offsprings += s->offsprings;
            }
            
            size_t over = allowed_offsprings - offsprings;
            
            while(over != 0) {
                for(Species* sp : species) {
                    if(over == 0) break;
                    ++sp->offsprings;
                    --over;
                }
            }
        }
        
        Network* reproduce() {
            map.clear();
            
            size_t size = networks.size();
            
            std::vector<Network> ns;
            
            for(Species* sp : species) {
                size_t spsize = sp->networks.size();
                for(size_t n = 0; n < sp->offsprings; ++n) {
                    Network baby;
                    
                    size_t i1 = random() * spsize;
                    
                    if(random() < mate_rate) {
                        if(random() < interspecies_mate_rate) {
                            size_t i2 = random() * size;
                            
                            Network::crossover(sp->networks[i1], networks[i2], &baby);
                        }else{
                            size_t i2 = random() * spsize;
                            
                            Network::crossover(sp->networks[i1], sp->networks[i2], &baby);
                        }
                    }else{
                        baby = *sp->networks[i1];
                    }
                    
                    if(random() < mutate_weights_rate) {
                        baby.mutate_weights();
                    }
                    
                    if(random() < mutate_topology_rate) {
                        baby.mutate_topology(&map, &innovation);
                    }
                    
                    ns.push_back(baby);
                }
            }
            
            size_t q = ns.size();
            std::sort(networks.data(), networks.data() + size, network_sort);
            
            for(size_t i = 0; i < q; ++i) {
                _remove(networks[size - i - 1]);
                *(networks[size - i - 1]) = ns[i];
                _add(networks[size - i - 1]);
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
        
        std::vector<Network*> networks;
        std::vector<Species*> species;
        
        float best_fitness;
        
    private:
        
        innov_map map;
        
        void _add(Network* n) {
            Species* sp = nullptr;
            float mc = similar_rate;
            
            for(Species* s : species) {
                Network* j = s->networks.front();
                float ts = Network::distance(n, j);
                if(ts < mc) {
                    mc = ts;
                    sp = s;
                }
            }
            
            if(sp == nullptr) {
                Species* sp = new Species();
                sp->networks.push_back(n);
                sp->time_since_improvement = 0;
                sp->max_fitness = -FLT_MAX;
                n->sp = sp;
                species.push_back(sp);
            }else{
                sp->networks.push_back(n);
                n->sp = sp;
            }
        }
        
        void _remove(Network* n) {
            std::vector<Network*>::iterator it = n->sp->networks.end();
            std::vector<Network*>::iterator b = n->sp->networks.begin();
            while(it-- != b) {
                if(*it == n) {
                    n->sp->networks.erase(it);
                    
                    if(n->sp->networks.empty()) {
                        std::vector<Species*>::iterator q = species.end();
                        std::vector<Species*>::iterator p = species.begin();
                        
                        while(q-- != p) {
                            if(*q == n->sp) {
                                species.erase(q);
                                delete n->sp;
                                break;
                            }
                        }
                    }
                    
                    break;
                }
            }
        }
        
        void _speciate() {
            species.clear();
            
            for(Network* n : networks) {
                _add(n);
            }
        }
        
    };
    
}

#endif /* Population_h */
