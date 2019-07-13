#include "aurora_physic_engine.h"

namespace aurora {

static Quantity solidNByVolume[Material::MaterialCount] =
{
    10, // Oxygen
    10, // Nitrogen
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

static Quantity liquidNByVolume[Material::MaterialCount] =
{
    10, // Oxygen
    10, // Nitrogen
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

static Scalar liquidThermalExpansionCoef[Material::MaterialCount] =
{
    0.01, // Oxygen
    0.01, // Nitrogen
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
    100., // Oxygen
    100., // Nitrogen
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

static Energy specifHeat[Material::MaterialCount] = // Joules.N-1.K-1
{
    1000, // Oxygen
    1000, // Nitrogen
    4000, // Water
    1000, // Methane
    1000, // CarbonDioxide
    1000, // Salt
    1000, // Iron
    1000, // Gold
    1000, // Limestone
    1000, // Lime
    1000, // Wood
    1000, // Charcoal
    1000, // Coal
    1000  // Clay
};

static Material dissolvedMaterial[Material::MaterialCount] = // Joules.N-1.K-1
{
    CarbonDioxide, // Oxygen
    CarbonDioxide, // Nitrogen
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


static Scalar GasConstant = 1e-5;

////////////////////////
/// PhysicalConstants
////////////////////////

Quantity PhysicalConstants::GetSolidNByVolume(Material material, Volume volume)
{
    return solidNByVolume[material] * volume;
}


Volume PhysicalConstants::GetSolidVolumeByN(Material material, Quantity N)
{
    Volume volume = N / solidNByVolume[material];
    if(volume == 0 && N > 0)
    {
        volume = 1;
    }

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

    if(volume == 0 && N > 0)
    {
        volume = 1;
    }

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
    Quantity N = Quantity(pressure * volume / (GasConstant * temperature));
    return N;
}


Material PhysicalConstants::GetDissolvedMaterial(Material material)
{
    return dissolvedMaterial[material];
}



////////////////////////
/// GasNode
////////////////////////

GasNode::GasNode()
    : m_altitude(0)
    , m_volume(0)
    , m_thermalEnergy(0)
{
    for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
    {
        m_nMaterials[gazIndex] = 0;
    }
}

void GasNode::SetVolume(Volume volume)
{
    m_volume = volume;
}


Volume GasNode::GetVolume() const
{
    return m_volume;
}

void GasNode::AddN(Material material, Quantity N)
{
    m_nMaterials[material] += N;
}

void GasNode::AddThermalEnergy(Energy thermalEnergy)
{
    m_thermalEnergy += thermalEnergy;
}

void GasNode::TakeN(Material material, Quantity N)
{
    m_nMaterials[material] -= N;
}

void GasNode::TakeThermalEnergy(Energy thermalEnergy)
{
    m_thermalEnergy -= thermalEnergy;
}

Quantity GasNode::GetN(Material material) const
{
    return m_nMaterials[material];
}

Energy GasNode::GetThermalEnergy() const
{
    return m_thermalEnergy;
}

////////////////////////
/// LiquidNode
////////////////////////

LiquidNode::LiquidNode(Material material)
    : m_material(material)
    , m_altitude(0)
    , m_volume(0)
    , m_N(0)
    , m_dissolvedN(0)
    , m_thermalEnergy(0)
{
}

void LiquidNode::SetVolume(Volume volume)
{
    m_volume = volume;
}


Volume LiquidNode::GetVolume() const
{
    return m_volume;
}

Material LiquidNode::GetMaterial() const
{
    return m_material;
}

Quantity LiquidNode::GetN() const
{
    return m_N;
}

Quantity LiquidNode::GetDissolvedN() const
{
    return m_dissolvedN;
}

Energy LiquidNode::GetThermalEnergy() const
{
    return m_thermalEnergy;
}

void LiquidNode::AddN(Quantity N)
{
    m_N += N;
}

void LiquidNode::AddDissolvedN(Quantity N)
{
    m_dissolvedN += N;
}

void LiquidNode::AddThermalEnergy(Energy thermalEnergy)
{
    m_thermalEnergy += thermalEnergy;
}


void LiquidNode::TakeN(Quantity N)
{
    m_N -= N;
}

void LiquidNode::TakeDissolvedN(Quantity N)
{
    m_dissolvedN -= N;
}

void LiquidNode::TakeThermalEnergy(Energy thermalEnergy)
{
    m_thermalEnergy -= thermalEnergy;
}



}

