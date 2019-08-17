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
    
    size_t i1 = sp->alive_after + (size_t)(ne_random() * sp->parents);
    
    if(ne_random() < params.mate_prob) {
        if(ne_random() < params.interspecies_mate_prob) {
            size_t i2 = alive_after + (size_t)(ne_random() * parents);
            ne_crossover(sp->genomes[i1], genomes[i2], baby, params);
        }else{
            size_t i2 = sp->alive_after + (size_t)(ne_random() * sp->parents);
            ne_crossover(sp->genomes[i1], sp->genomes[i2], baby, params);
        }
    }else{
        *baby = *sp->genomes[i1];
    }
    
    ne_mutate(baby, params, &map, &innovation);
    
    return baby;
}

ne_genome* ne_population::select() {
    _speciate();
    
    size_t idx = 0;
    for(size_t i = 0; i < params.population; ++i) {
        if(isnan(genomes[i]->fitness))
            genomes[i]->fitness = 0.0;
        
        if(genomes[i]->fitness > genomes[idx]->fitness)
            idx = i;
    }
    
    size_t ss = species.size();
    
    if(params.num_of_species != 0) {
        if(ss > params.num_of_species) {
            params.species_thresh += params.species_mod;
        }else if(ss < params.num_of_species) {
            if(params.species_thresh > params.species_mod)
                params.species_thresh -= params.species_mod;
        }
    }
    
    size_t offsprings = 0;
    alive_after = 0;
    
    double total_rank = 0.0;
    
    for(ne_species* sp : species) {
        size_t spsize = sp->genomes.size();
        double inv_size = 1.0 / (double) spsize;
        
        std::sort(sp->genomes.data(), sp->genomes.data() + spsize, ne_genome_sort);
        
        sp->alive_after = (size_t)(spsize * params.kill_ratio);
        sp->parents = spsize - sp->alive_after;
        
        alive_after += sp->alive_after;
        
        sp->avg_fitness = 0.0;
        
        for(size_t i = 0; i < spsize; ++i) {
            if(i >= sp->alive_after)
                sp->avg_fitness += std::max(0.0, sp->genomes[i]->fitness);
            
            sp->genomes[i]->adjusted_fitness = sp->genomes[i]->fitness * inv_size;
        }
        
        sp->avg_fitness /= (double) sp->parents;
        
        total_rank += sp->avg_fitness;
    }
    
    if(total_rank == 0.0)
        total_rank = 1.0;
    
    parents = params.population - alive_after;
    
    for(ne_species* sp : species) {
        sp->offsprings = (size_t)(params.population * (sp->avg_fitness / total_rank));
        offsprings += sp->offsprings;
    }
    
    std::sort(species.data(), species.data() + ss, ne_species_sort);
    
    size_t leftover = params.population - offsprings;
    while(leftover != 0) {
        for(ne_species* sp : species) {
            if(leftover == 0) break;
            ++sp->offsprings;
            --leftover;
        }
    }
    
    return genomes[idx];
}

void ne_population::reproduce() {
    map.clear();
    
    std::sort(genomes.data(), genomes.data() + params.population, ne_genome_adjusted_sort);
    
    std::vector<ne_genome*> ns;
    
    for(ne_species* sp : species) {
        if(sp->offsprings == 0) continue;
        
        for(size_t n = 1; n != sp->offsprings; ++n) {
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
