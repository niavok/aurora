#ifndef AURORA_PHYSIC_TYPES_H
#define AURORA_PHYSIC_TYPES_H

//#include "core/math/vector2.h"
//#include "core/math/rect2.h"
#include "../tools/aurora_types.h"
//#include <vector>

namespace aurora {

enum Material
{
    Hydrogen, //H2
    Nitrogen, // N2
    Oxygen, // O2
    Water,  // H2O
    Methane,// CH4
    CarbonDioxide, // CO2

    // Liquid only

    Salt, // NaCl
    Iron, // Fe
    Gold, // Au

    // Solid only
    Limestone, // CaCO3 -> Decay in CaO + CO2 at 800 C
    Lime, // CaO, used to do concrete ?
    Wood, // Decay at 400 C into 25% of Charcoal, 25% of Methane and 50 % of water
    Charcoal, // C
    Coal, // C + Sulfur, decay as coke at  1200C
    Clay, // Al2O3 2SiO2 2H2O

    MaterialCount,
    GasMoleculeCount = Salt,
};


}

#endif
