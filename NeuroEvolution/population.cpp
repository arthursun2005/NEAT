//
//  ne_population.cpp
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/11/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

ne_population& ne_population::operator = (const ne_population& population) {
    params = population.params;
    
    _kill();
    
    genomes.resize(params.population);
    
    for(uint64 i = 0; i < params.population; ++i) {
        genomes[i] = new ne_genome(*population.genomes[i]);
    }
    
    innovation = population.innovation;
    set = population.set;
    node_ids = population.node_ids;
    
    _speciate();
    
    return *this;
}

void ne_population::reset(const ne_params& _params, uint64 input_size, uint64 output_size) {
    params = _params;
    
    _kill();
    
    genomes.resize(params.population);
    
    innovation = 0;
    set.clear();
    
    for(ne_genome*& genome : genomes) {
        genome = new ne_genome();
        genome->reset(input_size, output_size, &set, &innovation);
    }
    
    node_ids = input_size + output_size + 1;
    
    _speciate();
}

ne_genome* ne_population::_breed(ne_species *sp) {
    ne_genome* baby;
    
    uint64 i1 = random(0, sp->parents);
    
    if(random(0.0, 1.0) < params.mutate_only_prob || sp->parents == 1) {
        baby = new ne_genome(*sp->genomes[i1]);
        
        ne_mutate(baby, &set, &innovation, &node_ids, params);
    }else{
        if(random(0.0, 1.0) < params.interspecies_mate_prob) {
            uint64 i2 = random(0, species.size());
            baby = ne_genome::crossover(sp->genomes[i1], species[i2]->genomes[0], params);
        }else{
            uint64 i2 = random(0, sp->parents);
            baby = ne_genome::crossover(sp->genomes[i1], sp->genomes[i2], params);
        }
        
        if(random(0.0, 1.0) >= params.mate_only_prob) {
            ne_mutate(baby, &set, &innovation, &node_ids, params);
        }
    }
    
    baby->eliminated = false;
    
    return baby;
}

ne_genome* ne_population::select() {
    uint64 offsprings = 0;
    
    float64 total_fitness = 0.0;
    
    ne_genome* best = genomes[0];

    for(ne_species* sp : species) {        
        std::sort(sp->genomes.data(), sp->genomes.data() + sp->genomes.size(), ne_genome::compare);
        
        if(sp->genomes[0]->fitness > best->fitness)
            best = sp->genomes[0];
    }
    
    for(ne_species* sp : species) {
        uint64 spsize = sp->genomes.size();
        
        sp->parents = (uint64)ceil(spsize * params.survive_thresh);
        
        sp->avg_fitness = 0.0;
        
        float64 bf = sp->genomes.front()->fitness;
        if(bf > sp->max_fitness) {
            sp->time_since_improvement = 0;
            sp->max_fitness = bf;
        }else{
            ++sp->time_since_improvement;
        }
        
        if(sp->time_since_improvement <= params.dropoff_age || bf == best->fitness) {
            for(uint64 i = 0; i < spsize; ++i) {
                if(isnan(sp->genomes[i]->fitness))
                    sp->genomes[i]->fitness = 0.0;
                
                if(i < sp->parents)
                    sp->avg_fitness += fmax(0.0, sp->genomes[i]->fitness);
            }
            
            sp->avg_fitness /= (float64) sp->parents;
        }
        
        total_fitness += sp->avg_fitness;
    }
    
    if(total_fitness != 0.0) {
        for(ne_species* sp : species) {
            sp->offsprings = (uint64)(params.population * (sp->avg_fitness / total_fitness));
            offsprings += sp->offsprings;
        }
        
        std::sort(species.data(), species.data() + species.size(), ne_species::compare);
    }
    
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
    std::vector<ne_genome*> babies;
    
    for(ne_species* sp : species) {
        for(ne_genome* g : sp->genomes) {
            g->eliminated = true;
        }
        
        if(sp->offsprings != 0) {
            for(uint64 n = 1; n != sp->offsprings; ++n) {
                babies.push_back(_breed(sp));
            }
            
            sp->genomes.front()->eliminated = false;
        }
    }
    
    for(ne_genome* g : babies) {
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
        float64 ts = ne_genome::distance(g, j, params);
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
