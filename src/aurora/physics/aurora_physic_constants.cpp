#include "aurora_physic_constants.h"

namespace aurora {

static Quantity solidNByVolume[Material::MaterialCount] = // TODO
{
    10, // Hydrogen
    10, // Nitrogen
    10, // Oxygen
    10, // Water
    10, // Methane
    10, // CarbonDioxide
    10, // Salt
    10, // Iron
    10, // Gold
    10, // Limestone
    10, // Lime
    10, // Wood
    10, // Charcoal
    10, // Coal
    10  // Clay
};

static Quantity liquidNByVolume[Material::MaterialCount] = // TODO
{
    10, // Hydrogen
    10, // Nitrogen
    10, // Oxygen
    10, // Methane
    10, // CarbonDioxide
    10, // Salt
    10, // Iron
    10, // Gold
    10, // Limestone
    10, // Lime
    10, // Wood
    10, // Charcoal
    10, // Coal
    10  // Clay
};

static Scalar liquidThermalExpansionCoef[Material::MaterialCount] = // TODO
{
    0.01, // Hydrogen
    0.01, // Nitrogen
    0.01, // Oxygen
    0.01, // Water
    0.01, // Methane
    0.01, // CarbonDioxide
    0.01, // Salt
    0.01, // Iron
    0.01, // Gold
    0.01, // Limestone
    0.01, // Lime
    0.01, // Wood
    0.01, // Charcoal
    0.01, // Coal
    0.01  // Clay
};

static Scalar liquidCompressibilityCoef[Material::MaterialCount] =
{
    100., // Hydrogen
    100., // Nitrogen
    100., // Oxygen
    100., // Water
    100., // Methane
    100., // CarbonDioxide
    100., // Salt
    100., // Iron
    100., // Gold
    100., // Limestone
    100., // Lime
    100., // Wood
    100., // Charcoal
    100., // Coal
    100.  // Clay
};

static Scalar specifHeat[Material::MaterialCount] = // Joules.N-1.K-1
{
    0.01, // Hydrogen
    0.01, // Nitrogen
    0.01, // Oxygen
    0.01, // Water
    0.01, // Methane
    0.01, // CarbonDioxide
    0.01, // Salt
    0.01, // Iron
    0.01, // Gold
    0.01, // Limestone
    0.01, // Lime
    0.01, // Wood
    0.01, // Charcoal
    0.01, // Coal
    0.01  // Clay
};

static Material dissolvedMaterial[Material::MaterialCount] = // Joules.N-1.K-1
{
    CarbonDioxide, // Hydrogen
    CarbonDioxide, // Nitrogen
    CarbonDioxide, // Oxygen
    CarbonDioxide, // Water
    CarbonDioxide, // Methane
    CarbonDioxide, // CarbonDioxide
    CarbonDioxide, // Salt
    CarbonDioxide, // Iron
    CarbonDioxide, // Gold
    CarbonDioxide, // Limestone
    CarbonDioxide, // Lime
    CarbonDioxide, // Wood
    CarbonDioxide, // Charcoal
    CarbonDioxide, // Coal
    CarbonDioxide  // Clay
};


static Scalar massByN[Material::MaterialCount] =
{
    0.11e-8, // Hydrogen
    1.55e-8, // Nitrogen
    1.77e-8, // Oxygen
    1.00e-8, // Water
    0.88e-8, // Methane
    2.44e-8, // CarbonDioxide
    10, // Salt // TODO
    10, // Iron
    10, // Gold
    10, // Limestone
    10, // Lime
    10, // Wood
    10, // Charcoal
    10, // Coal
    10  // Clay
};



//static Scalar SizeToSI = 1e-3;
//static Scalar VolumeToSI = SizeToSI * SizeToSI;

////////////////////////
/// PhysicalConstants
////////////////////////

Quantity PhysicalConstants::EstimateSolidNByVolume(Material material, Volume volume)
{
    return Quantity(solidNByVolume[material] * volume);
}


Volume PhysicalConstants::GetSolidVolumeByN(Material material, Quantity N)
{
    Volume volume = N / solidNByVolume[material];
    return volume;
}

Quantity PhysicalConstants::EstimateLiquidNByVolume(Material material, Volume volume, Scalar pressure, Scalar temperature)
{
    Quantity N0 = liquidNByVolume[material];
    Scalar thermalExpansionCoef = liquidThermalExpansionCoef[material]; // Volume multiplier by K
    Scalar compressibilityCoef = liquidCompressibilityCoef[material];

    Scalar ooStateCoef = (compressibilityCoef + pressure) / ((1. + temperature * thermalExpansionCoef) * compressibilityCoef);

    Quantity N = Quantity(volume * N0 * ooStateCoef);

    return N;
}

Volume PhysicalConstants::GetLiquidVolumeByN(Material material, Quantity N, Scalar pressure, Scalar temperature)
{
    Scalar V0 = 1./liquidNByVolume[material];
    Scalar thermalExpansionCoef = liquidThermalExpansionCoef[material]; // Volume multiplier by K
    Scalar compressibilityCoef = liquidCompressibilityCoef[material];

    Scalar stateCoef = (1. + temperature * thermalExpansionCoef) * compressibilityCoef / (compressibilityCoef + pressure);

    Volume volume = Volume(N * V0 * stateCoef);

    return volume;
}


Energy PhysicalConstants::EstimateThermalEnergy(Material material, Quantity N, Scalar temperature)
{
    return Energy(specifHeat[material] * N * temperature);
}

Quantity PhysicalConstants::EstimateGasN(Volume volume, Scalar pressure, Scalar temperature)
{
    // PV = NRT
    // N = PV / RT
    Quantity N = Quantity(pressure * volume / (gasConstant * temperature));
    return N;
}

Material PhysicalConstants::GetDissolvedMaterial(Material material)
{
    return dissolvedMaterial[material];
}

Scalar PhysicalConstants::GetSpecificHeat(Material material)
{
    return specifHeat[material];
}

Scalar PhysicalConstants::GetMassByN(Material material)
{
    return massByN[material];
}

}