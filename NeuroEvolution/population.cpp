//
//  ne_population.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/11/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

ne_genome* ne_population::_breed(ne_species *sp) {
    ne_genome* baby;
    
    uint64 i1 = (uint64)(ne_random() * sp->parents);
    
    if(ne_random() < params.mate_prob) {
        if(ne_random() < params.interspecies_mate_prob) {
            uint64 i2 = (uint64)(ne_random() * parents);
            baby = ne_crossover(sp->genomes[i1], genomes[i2], params);
        }else{
            uint64 i2 = (uint64)(ne_random() * sp->parents);
            baby = ne_crossover(sp->genomes[i1], sp->genomes[i2], params);
        }
    }else{
        baby = new ne_genome(*sp->genomes[i1]);
    }
    
    ne_mutate(baby, &map, &node_ids, &innovation, params);
    
    baby->eliminated = false;
    
    return baby;
}

ne_genome* ne_population::select() {    
    uint64 ss = species.size();
    
    uint64 offsprings = 0;
    parents = 0;
    
    float64 total_rank = 0.0;
    
    ne_genome* best = genomes[0];
    for(uint64 i = 0; i < params.population; ++i) {
        if(genomes[i]->fitness > best->fitness)
            best = genomes[i];
    }
    
    for(ne_species* sp : species) {
        uint64 spsize = sp->genomes.size();
        float64 inv_size = 1.0 / (float64) spsize;
        
        std::sort(sp->genomes.data(), sp->genomes.data() + spsize, ne_genome_sort);
        
        sp->parents = (uint64)ceil(spsize * params.survive_thresh);
        
        parents += sp->parents;
        
        sp->avg_fitness = 0.0;
        
        for(uint64 i = 0; i < spsize; ++i) {
            if(isnan(sp->genomes[i]->fitness))
                sp->genomes[i]->fitness = 0.0;
            
            if(i < sp->parents)
                sp->avg_fitness += fmax(0.0, sp->genomes[i]->fitness);
            
            sp->genomes[i]->adjusted_fitness = sp->genomes[i]->fitness * inv_size;
        }
        
        sp->avg_fitness /= (float64) sp->parents;
        
        float64 bf = sp->genomes.front()->fitness;
        if(bf > sp->max_fitness) {
            sp->time_since_improvement = 0;
            sp->max_fitness = bf;
        }else{
            ++sp->time_since_improvement;
        }
        
        if(sp->time_since_improvement >= params.dropoff_age && bf < best->fitness) {
            sp->avg_fitness = 0.0;
            
            for(ne_genome* g : sp->genomes) {
                g->adjusted_fitness = 0.0;
            }
        }
        
        total_rank += sp->avg_fitness;
    }
    
    if(total_rank == 0.0)
        total_rank = 1.0;
    
    for(ne_species* sp : species) {
        sp->offsprings = (uint64)(params.population * (sp->avg_fitness / total_rank));
        offsprings += sp->offsprings;
    }
    
    std::sort(species.data(), species.data() + ss, ne_species_sort);
    
    uint64 leftover = params.population - offsprings;
    while(leftover != 0) {
        for(ne_species* sp : species) {
            if(leftover == 0) break;
            ++sp->offsprings;
            --leftover;
        }
    }
    
    return best;
}

void ne_population::reproduce() {
    map.clear();
    
    std::sort(genomes.data(), genomes.data() + params.population, ne_genome_adjusted_sort);
    
    std::vector<ne_genome*> ns;
    
    for(ne_species* sp : species) {
        for(ne_genome* g : sp->genomes) {
            g->eliminated = true;
        }
        
        if(sp->offsprings != 0) {
            for(uint64 n = 1; n != sp->offsprings; ++n) {
                ns.push_back(_breed(sp));
            }
            
            sp->genomes.front()->eliminated = false;
        }
    }
    
    for(ne_genome* g : ns) {
        _add(g);
        genomes.push_back(g);
    }
    
    std::vector<ne_genome*>::iterator begin, end;
    std::vector<ne_species*>::iterator p = species.begin(), q = species.end();
    
    while(q-- != p) {
        begin = (*q)->genomes.begin();
        end = (*q)->genomes.end();
        
        while(end-- != begin) {
            if((*end)->eliminated)
                (*q)->genomes.erase(end);
        }
        
        if((*q)->genomes.empty()) {
            delete *q;
            species.erase(q);
        }
    }
    
    begin = genomes.begin();
    end = genomes.end();
    
    while(end-- != begin) {
        if((*end)->eliminated) {
            delete *end;
            genomes.erase(end);
        }
    }
}

void ne_population::_speciate() {
    for(ne_species* sp : species) {
        delete sp;
    }
    
    species.clear();
    
    for(ne_genome* g : genomes) {
        _add(g);
    }
}

void ne_population::_add(ne_genome *g) {
    ne_species* sp = nullptr;
    float64 mc = params.compat_thresh;
    
    for(ne_species* s : species) {
        ne_genome* j = s->genomes.front();
        float64 ts = ne_distance(g, j, params);
        if(ts < mc) {
            mc = ts;
            sp = s;
        }
    }
    
    if(sp == nullptr) {
        sp = new ne_species();
        species.push_back(sp);
    }
    
    sp->genomes.push_back(g);
}

void ne_population::_kill() {
    for(ne_genome* g : genomes) {
        delete g;
    }
    
    for(ne_species* sp : species) {
        delete sp;
    }
    
    genomes.clear();
    species.clear();
}
