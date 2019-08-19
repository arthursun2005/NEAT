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
    
    uint64 i1 = sp->alive_after + (uint64)(ne_random() * sp->parents);
    
    if(ne_random() < params.mate_prob) {
        if(ne_random() < params.interspecies_mate_prob) {
            uint64 i2 = alive_after + (uint64)(ne_random() * parents);
            baby = ne_crossover(sp->genomes[i1], genomes[i2], params);
        }else{
            uint64 i2 = sp->alive_after + (uint64)(ne_random() * sp->parents);
            baby = ne_crossover(sp->genomes[i1], sp->genomes[i2], params);
        }
    }else{
        baby = new ne_genome(*sp->genomes[i1]);
    }
    
    ne_mutate(baby, &map, &node_ids, &innovation, params);
    
    return baby;
}

ne_genome* ne_population::select() {
    _speciate();
    
    uint64 ss = species.size();
    
    if(params.num_of_species != 0) {
        if(ss > params.num_of_species) {
            params.compat_thresh += params.compat_mod;
        }else if(ss < params.num_of_species) {
            if(params.compat_thresh > params.compat_mod)
                params.compat_thresh -= params.compat_mod;
        }
    }
    
    uint64 offsprings = 0;
    alive_after = 0;
    
    float64 total_rank = 0.0;
    
    std::sort(genomes.data(), genomes.data() + params.population, ne_genome_sort);
    
    float64 inv_size = 1.0 / (float64) params.population;
    for(uint64 i = 0; i < params.population; ++i) {
        genomes[i]->rank = i * inv_size;
    }
    
    for(ne_species* sp : species) {
        uint64 spsize = sp->genomes.size();
        float64 inv_size = 1.0 / (float64) spsize;
        
        std::sort(sp->genomes.data(), sp->genomes.data() + spsize, ne_genome_sort);
        
        sp->alive_after = (uint64)(spsize * params.kill_ratio);
        sp->parents = spsize - sp->alive_after;
        
        alive_after += sp->alive_after;
        
        sp->avg_fitness = 0.0;
        sp->rank = 0.0;
        
        for(uint64 i = 0; i < spsize; ++i) {
            sp->rank += sp->genomes[i]->rank;
            sp->avg_fitness += sp->genomes[i]->fitness;
            
            sp->genomes[i]->adjusted_fitness = sp->genomes[i]->fitness * inv_size;
        }
        
        sp->rank *= inv_size;
        sp->avg_fitness *= inv_size;
        
        total_rank += sp->rank;
    }
    
    parents = params.population - alive_after;
    
    for(ne_species* sp : species) {
        sp->offsprings = (uint64)(params.population * (sp->rank / total_rank));
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
    
    return genomes.back();
}

void ne_population::reproduce() {
    map.clear();
    
    std::sort(genomes.data(), genomes.data() + params.population, ne_genome_adjusted_sort);
    
    std::vector<ne_genome*> ns;
    
    for(ne_species* sp : species) {
        if(sp->offsprings == 0) continue;
        
        for(uint64 n = 1; n != sp->offsprings; ++n) {
            ns.push_back(_breed(sp));
        }
        
        ns.push_back(new ne_genome(*sp->genomes.back()));
    }
    
    _kill();
    
    genomes = ns;
}

void ne_population::_speciate() {
    for(ne_genome* g : genomes) {
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
