//
//  ne_population.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/11/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

ne_genome* ne_population::_breed(ne_species *sp) {
    ne_genome* baby = new ne_genome();
    
    size_t size = genomes.size();
    size_t spsize = sp->genomes.size();
    
    size_t i1 = ne_random() * spsize;
    
    if(ne_random() < params.mate_prob && spsize != 1) {
        if(ne_random() < params.interspecies_mate_prob) {
            size_t i2 = ne_random() * size;
            ne_crossover(sp->genomes[i1], genomes[i2], baby, params);
        }else{
            size_t i2 = ne_random() * spsize;
            ne_crossover(sp->genomes[i1], sp->genomes[i2], baby, params);
        }
    }else{
        *baby = *sp->genomes[i1];
    }
    
    ne_mutate(baby, params, &map, &innovation);
    
    return baby;
}

void ne_population::_kill() {
    std::vector<ne_genome*>::iterator it = genomes.end();
    std::vector<ne_genome*>::iterator begin = genomes.begin();
    
    while(it-- != begin) {
        if((*it)->killed) {
            _remove(*it);
            delete *it;
            genomes.erase(it);
        }
    }
}

ne_genome* ne_population::select() {
    size_t offsprings = 0;
    
    double total_rank = 0.0;
    
    for(ne_species* sp : species) {
        size_t spsize = sp->genomes.size();
        
        std::sort(sp->genomes.data(), sp->genomes.data() + spsize, ne_genome_sort);
        
        size_t midpoint = floorf(spsize * params.kill_ratio);
        
        for(size_t i = 0; i != midpoint; ++i) {
            sp->genomes[i]->killed = true;
        }
        
        sp->avg_fitness = 0.0;
        
        for(ne_genome* g : sp->genomes) {
            if(isnan(g->fitness) || g->fitness < 0.0)
                g->fitness = 0.0;
            
            sp->avg_fitness += g->fitness;
        }
        
        sp->avg_fitness /= (double) spsize;
        
        total_rank += sp->avg_fitness;
    }
    
    for(ne_species* sp : species) {
        sp->offsprings = (size_t)(params.population * sp->avg_fitness / total_rank);
        offsprings += sp->offsprings;
    }
    
    std::sort(species.data(), species.data() + species.size(), ne_species_sort);
    
    size_t idx = 0;
    for(size_t i = 0; i < params.population; ++i)
        if(genomes[i]->fitness > genomes[idx]->fitness)
            idx = i;
    
    return genomes[idx];
}

void ne_population::reproduce() {
    _kill();
    
    map.clear();
    
    std::vector<ne_genome*> ns;
    
    for(ne_species* sp : species) {
        size_t n = sp->genomes.size();
        
        if(sp->offsprings != 0) {
            for(size_t n = 1; n != sp->offsprings; ++n) {
                ns.push_back(_breed(sp));
            }
            
            --n;
        }
        
        for(size_t i = 0; i != n; ++i) {
            sp->genomes[i]->killed = true;
        }
    }
    
    _kill();
        
    size_t size = genomes.size();
    size_t ss = species.size();
    size_t i = 0;
    
    while(size + ns.size() != params.population) {
        ne_species* sp = species[i];
        ns.push_back(_breed(sp));
        
        i = (i + 1) % ss;
    }
    
    for(ne_genome* g : ns) {
        genomes.push_back(g);
        _add(g);
    }
}

void ne_population::_add(ne_genome* g) {
    ne_species* sp = nullptr;
    double mc = params.species_thresh;
    
    for(ne_species* s : species) {
        ne_genome* j = s->genomes.front();
        double ts = ne_distance(g, j, params);
        if(ts < mc) {
            mc = ts;
            sp = s;
        }
    }
    
    if(sp == nullptr) {
        sp = new ne_species();
        species.push_back(sp);
    }
    
    g->sp = sp;
    sp->genomes.push_back(g);
}

void ne_population::_remove(ne_genome* g) {
    std::vector<ne_genome*>::iterator it = g->sp->genomes.end();
    std::vector<ne_genome*>::iterator b = g->sp->genomes.begin();
    while(it-- != b) {
        if(*it == g) {
            g->sp->genomes.erase(it);
            
            if(g->sp->genomes.empty()) {
                std::vector<ne_species*>::iterator q = species.end();
                std::vector<ne_species*>::iterator p = species.begin();
                
                while(q-- != p) {
                    if(*q == g->sp) {
                        species.erase(q);
                        delete g->sp;
                        break;
                    }
                }
            }
            
            break;
        }
    }
}

