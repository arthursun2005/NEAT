//
//  Species.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 8/5/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef Species_h
#define Species_h

#include "Network.h"

namespace NE {

    struct Species
    {
        std::vector<Network*> networks;
    };
    
}

#endif /* Species_h */
