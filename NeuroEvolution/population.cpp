//
//  ne_population.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/11/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

ne_network* ne_population::_breed(ne_species *sp) {
    ne_network* baby = new ne_network();
    
    size_t i1 = sp->midpoint + (size_t)(ne_random() * sp->alive_size);
    
    if(ne_random() < params.mate_prob) {
        if(ne_random() < params.interspecies_mate_prob) {
            size_t i2 = midpoint + (size_t)(ne_random() * alive_size);
            ne_network::crossover(sp->networks[i1], networks[i2], baby, params);
        }else{
            size_t i2 = sp->midpoint + (size_t)(ne_random() * sp->alive_size);
            ne_network::crossover(sp->networks[i1], sp->networks[i2], baby, params);
        }
    }else{
        *baby = *sp->networks[i1];
    }
    
    ne_mutate_network(baby, params, &map, &innovation);
    
    return baby;
}

ne_network* ne_population::select() {
    size_t size = networks.size();
    
    size_t offsprings = 0;
    
    float total_rank = 0.0f;
    
    best_fitness = -FLT_MAX;
    
    midpoint = 0;
    
    for(ne_species* sp : species) {
        size_t spsize = sp->networks.size();
        float inv_size = 1.0f / (float) spsize;
        
        std::sort(sp->networks.data(), sp->networks.data() + spsize, ne_network_sort_fitness);
        
        sp->midpoint = spsize / 2;
        sp->alive_size = spsize - sp->midpoint;
        
        for(size_t i = 0; i != sp->midpoint; ++i) {
            sp->networks[i]->adjusted_fitness = 0.0f;
            ++midpoint;
        }
        
        for(size_t i = sp->midpoint; i != spsize; ++i) {
            sp->networks[i]->adjusted_fitness = sp->networks[i]->fitness * inv_size;
        }
    }
    
    std::sort(networks.data(), networks.data() + size, ne_network_sort_adjusted_fitness);
    
    float rank = 0.0f;
    float last_fitness = -FLT_MAX;
    for(size_t i = 0; i < size; ++i) {
        if(networks[i]->adjusted_fitness > last_fitness) {
            rank += 1.0f;
        }
        
        networks[i]->rank = rank;
    }
    
    for(ne_species* sp : species) {
        sp->avg_fitness = 0.0f;
        sp->best_fitness = -FLT_MAX;
        sp->rank = 0.0f;
        
        float inv_size = 1.0f / (float) sp->networks.size();
        
        for(ne_network* n : sp->networks) {
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
        
        sp->avg_fitness *= inv_size;
        sp->rank *= inv_size;
    }
    
    for(ne_species* sp : species) {
        if(sp->time_since_improvement >= params.dead_species_age && sp->best_fitness < best_fitness) {
            size_t spsize = sp->networks.size();
            for(size_t i = sp->midpoint; i != spsize; ++i) {
                sp->networks[i]->adjusted_fitness = 0.0f;
                ++midpoint;
            }
            
            sp->rank = 0.0f;
        }
        
        total_rank += sp->rank;
    }
    
    std::sort(species.data(), species.data() + species.size(), ne_species_sort);
    
    for(ne_species* sp : species) {
        sp->offsprings = floorf(midpoint * (sp->rank / total_rank));
        offsprings += sp->offsprings;
    }
    
    size_t over = midpoint - offsprings;
    
    while(over != 0) {
        for(ne_species* sp : species) {
            if(over == 0) break;
            ++sp->offsprings;
            --over;
        }
    }
    
    for(ne_species* sp : species) {
        if(sp->offsprings == 0) {
            for(ne_network* n : sp->networks) {
                n->rank = 0.0f;
            }
        }
    }
    
    alive_size = size - midpoint;
    
    return networks.back();
}

void ne_population::reproduce() {
    map.clear();
    
    size_t size = networks.size();
    
    std::vector<ne_network*> ns;
    
    for(ne_species* sp : species) {
        for(size_t n = 0; n != sp->offsprings; ++n) {
            ns.push_back(_breed(sp));
        }
    }
    
    size_t q = ns.size();
    std::sort(networks.data(), networks.data() + size, ne_network_sort_adjusted_fitness);
    
    for(size_t i = 0; i < q; ++i) {
        ne_network*& net = networks[i];
        
        _remove(net);
        delete net;
        
        net = ns[i];
        _add(net);
    }
    
    for(size_t i = 0; i < size; ++i) {
        ++networks[i]->age;
    }
}

void ne_population::_add(ne_network* n) {
    ne_species* sp = nullptr;
    float mc = params.species_thresh;
    
    for(ne_species* s : species) {
        ne_network* j = s->networks.front();
        float ts = ne_network::distance(n, j, params);
        if(ts < mc) {
            mc = ts;
            sp = s;
        }
    }
    
    if(sp == nullptr) {
        sp = new ne_species();
        sp->time_since_improvement = 0;
        sp->max_fitness = -FLT_MAX;
        species.push_back(sp);
    }
    
    n->sp = sp;
    sp->networks.push_back(n);
}

void ne_population::_remove(ne_network* n) {
    std::vector<ne_network*>::iterator it = n->sp->networks.end();
    std::vector<ne_network*>::iterator b = n->sp->networks.begin();
    while(it-- != b) {
        if(*it == n) {
            n->sp->networks.erase(it);
            
            if(n->sp->networks.empty()) {
                std::vector<ne_species*>::iterator q = species.end();
                std::vector<ne_species*>::iterator p = species.begin();
                
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

