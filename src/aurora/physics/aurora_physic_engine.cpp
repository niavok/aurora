#include "aurora_physic_engine.h"

#include <assert.h>

namespace aurora {

static const Scalar gravity = 10; // m.s-2

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

static Energy specifHeat[Material::MaterialCount] = // Joules.N-1.K-1
{
    1000, // Hydrogen
    1000, // Nitrogen
    1000, // Oxygen
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


static Scalar GasConstant = 1e-5;

//static Scalar SizeToSI = 1e-3;
//static Scalar VolumeToSI = SizeToSI * SizeToSI;

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

GasNode::GasNode(GasNode& node)
    : m_altitude(node.m_altitude)
    , m_volume(node.m_volume)
    , m_thermalEnergy(node.m_thermalEnergy)
    , m_cacheComputed(false)
{
    for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
    {
        m_nMaterials[gazIndex] = node.m_nMaterials[gazIndex];
    }
}

void GasNode::SetVolume(Volume volume)
{
    m_volume = volume;
    m_cacheComputed = false;
}


Volume GasNode::GetVolume() const
{
    return m_volume;
}

void GasNode::AddN(Material material, Quantity N)
{
    m_nMaterials[material] += N;
    m_cacheComputed = false;
}

void GasNode::AddThermalEnergy(Energy thermalEnergy)
{
    m_thermalEnergy += thermalEnergy;
    m_cacheComputed = false;
}

void GasNode::TakeN(Material material, Quantity N)
{
    m_nMaterials[material] -= N;
    m_cacheComputed = false;
}

void GasNode::TakeThermalEnergy(Energy thermalEnergy)
{
    m_thermalEnergy -= thermalEnergy;
    m_cacheComputed = false;
}

Quantity GasNode::GetN(Material material) const
{
    return m_nMaterials[material];
}

Energy GasNode::GetThermalEnergy() const
{
    return m_thermalEnergy;
}

void GasNode::ComputeCache()
{
    // Compute N, energy and mass
    m_cacheN = 0;
    m_cacheMass = 0;
    Energy energyPerK = 0;
    for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
    {
        Quantity materialN = m_nMaterials[gazIndex];

        m_cacheN += materialN;
        energyPerK += specifHeat[gazIndex] * materialN;

        m_cacheMass += massByN[gazIndex] * materialN;
    }

    // Compute temperature
    if(energyPerK == 0)
    {
        m_cacheTemperature = 0;
    }
    else
    {
        m_cacheTemperature = m_thermalEnergy / energyPerK;
    }

    // Compute pressure
    m_cachePressure = m_cacheN * GasConstant * m_cacheTemperature / m_volume;

    Scalar density = Scalar(m_cacheMass) / m_volume;
    m_cachePressureGradient = density * gravity;

    m_cacheComputed = true;
}



Scalar GasNode::GetPressure() const
{
    assert(m_cacheComputed);
    return m_cachePressure;
}

Scalar GasNode::GetTemperature() const
{
    assert(m_cacheComputed);
    return m_cacheTemperature;
}

Quantity GasNode::GetN() const
{
    assert(m_cacheComputed);
    return m_cacheN;
}

Scalar GasNode::GetPressureGradient() const
{
    assert(m_cacheComputed);
    return m_cachePressureGradient;
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

