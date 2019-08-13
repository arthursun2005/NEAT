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
    
    size_t size = networks.size();
    size_t spsize = sp->networks.size();
    
    size_t i1 = ne_random() * spsize;
    
    if(ne_random() < params.mate_prob && spsize != 1) {
        if(ne_random() < params.interspecies_mate_prob) {
            size_t i2 = ne_random() * size;
            ne_network::crossover(sp->networks[i1], networks[i2], baby, params);
        }else{
            size_t i2 = ne_random() * spsize;
            ne_network::crossover(sp->networks[i1], sp->networks[i2], baby, params);
        }
    }else{
        *baby = *sp->networks[i1];
    }
    
    ne_mutate_network(baby, params, &map, &innovation);
    
    return baby;
}

void ne_population::_kill() {
    std::vector<ne_network*>::iterator it = networks.end();
    std::vector<ne_network*>::iterator begin = networks.begin();
    
    while(it-- != begin) {
        if((*it)->killed) {
            _remove(*it);
            networks.erase(it);
        }
    }
}

ne_network* ne_population::select() {
    size_t size = networks.size();
    
    size_t offsprings = 0;
    
    float total_rank = 0.0f;
    
    for(ne_species* sp : species) {
        size_t spsize = sp->networks.size();
        float inv_size = 1.0f / (float) spsize;
        
        std::sort(sp->networks.data(), sp->networks.data() + spsize, ne_network_sort_fitness);
        
        size_t midpoint = spsize * params.kill_ratio;
        
        for(size_t i = 0; i != midpoint; ++i) {
            sp->networks[i]->adjusted_fitness = sp->networks[i]->fitness * inv_size;
            sp->networks[i]->killed = true;
        }
        
        for(size_t i = midpoint; i != spsize; ++i) {
            sp->networks[i]->adjusted_fitness = sp->networks[i]->fitness * inv_size;
        }
    }
    
    std::sort(networks.data(), networks.data() + size, ne_network_sort_adjusted_fitness);
    
    size_t idx = 0;
    for(size_t i = 0; i < size; ++i) {
        if(networks[i]->fitness > networks[idx]->fitness) {
            idx = i;
        }
        
        networks[i]->rank = (float) i;
    }
    
    for(ne_species* sp : species) {
        sp->avg_fitness = 0.0f;
        sp->rank = 0.0f;
        
        float inv_size = 1.0f / (float) sp->networks.size();
        
        for(ne_network* n : sp->networks) {
            sp->avg_fitness += n->fitness;
            sp->rank += n->rank;
        }
        
        sp->avg_fitness *= inv_size;
        sp->rank *= inv_size;
        
        total_rank += sp->rank;
    }
    
    std::sort(species.data(), species.data() + species.size(), ne_species_sort);
    
    size_t expected_offsprings = size * params.kill_ratio;
    
    for(ne_species* sp : species) {
        sp->offsprings = floorf(expected_offsprings * (sp->rank / total_rank));
        offsprings += sp->offsprings;
    }
    
    size_t over = expected_offsprings - offsprings;
    
    while(over != 0) {
        for(ne_species* sp : species) {
            if(over == 0) break;
            ++sp->offsprings;
            --over;
        }
    }
    
    return networks[idx];
}

void ne_population::reproduce() {
    _kill();
    
    map.clear();
    
    std::vector<ne_network*> ns;
    
    for(ne_species* sp : species) {
        for(size_t n = 0; n != sp->offsprings; ++n) {
            ns.push_back(_breed(sp));
        }
        
        size_t n = sp->networks.size() - 1;
        for(size_t i = 0; i != n; ++i) {
            sp->networks[i]->killed = true;
        }
    }
    
    _kill();
    
    size_t size = networks.size();
    
    size_t ss = species.size();
    
    while(size + ns.size() < params.population) {
        ne_species* sp = species[rand64() % ss];
        ns.push_back(_breed(sp));
    }
    
    for(ne_network* network : ns) {
        networks.push_back(network);
        _add(network);
    }
    
    for(ne_network* network : networks) {
        ++network->age;
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

