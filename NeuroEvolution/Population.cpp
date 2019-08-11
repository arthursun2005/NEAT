//
//  Population.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/11/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "Population.h"

namespace NE {
    
    Network* Population::select() {
        size_t size = networks.size();
        
        size_t offsprings = 0;
        size_t allowed_offsprings = roundf(size * (1.0f - survival_thresh));
        float total_rank = 0.0f;
        
        best_fitness = -FLT_MAX;
        
        std::sort(networks.data(), networks.data() + size, network_sort);
        
        for(size_t i = 0; i < size; ++i) {
            networks[i]->rank = (float) i;
        }
        
        for(Species* sp : species) {
            sp->avg_fitness = 0.0f;
            sp->best_fitness = -FLT_MAX;
            sp->rank = 0.0f;
            
            for(Network* n : sp->networks) {
                sp->best_fitness = std::max(sp->best_fitness, n->fitness);
                sp->avg_fitness += n->fitness;
                sp->rank += n->rank;
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
            
            float spsize = (float) sp->networks.size();
            
            sp->avg_fitness /= spsize;
            sp->rank /= spsize;
        }
        
        for(Species* sp : species) {
            if(sp->time_since_improvement >= dead_species_age && sp->best_fitness < best_fitness) {
                for(Network* n : sp->networks) {
                    n->rank = 0.0f;
                }
                sp->rank = 0.0f;
            }
            
            total_rank += sp->rank;
        }
        
        std::sort(species.data(), species.data() + species.size(), species_sort);
        
        for(Species* sp : species) {
            sp->offsprings = floorf(allowed_offsprings * (sp->rank / total_rank));
            offsprings += sp->offsprings;
        }
        
        size_t over = allowed_offsprings - offsprings;
        
        while(over != 0) {
            for(Species* sp : species) {
                if(over == 0) break;
                ++sp->offsprings;
                --over;
            }
        }
        
        for(Species* sp : species) {
            if(sp->offsprings == 0) {
                for(Network* n : sp->networks) {
                    n->rank = 0.0f;
                }
            }
        }
        
        return networks.back();
    }

    void Population::reproduce() {
        map.clear();
        
        size_t size = networks.size();
        
        std::vector<Network*> ns;
        
        for(Species* sp : species) {
            size_t spsize = sp->networks.size();
            
            for(size_t n = 0; n < sp->offsprings; ++n) {
                Network* baby = new Network();
                
                size_t i1 = random() * spsize;
                
                if(random() < mate_prob) {
                    if(random() < interspecies_mate_prob) {
                        size_t i2 = random() * size;
                        Network::crossover(sp->networks[i1], networks[i2], baby);
                    }else{
                        size_t i2 = random() * spsize;
                        Network::crossover(sp->networks[i1], sp->networks[i2], baby);
                    }
                }else{
                    *baby = *sp->networks[i1];
                }
                
                if(random() < mutate_weights_prob) {
                    baby->mutate_weights();
                }
                
                if(random() < new_node_prob) {
                    baby->mutate_topology_add_node(&map, &innovation);
                }
                
                if(random() < new_link_prob) {
                    baby->mutate_topology_add_link(&map, &innovation);
                }
                
                if(random() < toggle_link_enable_prob) {
                    baby->mutate_toggle_link_enable(1);
                }
                
                if(random() < toggle_node_enable_prob) {
                    baby->mutate_toggle_node_enable(1);
                }
                
                ns.push_back(baby);
            }
        }
        
        size_t q = ns.size();
        std::sort(networks.data(), networks.data() + size, network_sort_rank);
        
        for(size_t i = 0; i < q; ++i) {
            Network*& net = networks[i];
            
            _remove(net);
            delete net;
            
            net = ns[i];
            _add(net);
        }
        
        for(size_t i = 0; i < size; ++i) {
            ++networks[i]->age;
        }
    }
    
    void Population::_add(Network* n) {
        Species* sp = nullptr;
        float mc = species_thresh;
        
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
    
    void Population::_remove(Network* n) {
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
    
}
