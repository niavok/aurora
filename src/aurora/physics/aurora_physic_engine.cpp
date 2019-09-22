#include "aurora_physic_engine.h"

#include <assert.h>
#include <stdio.h>

namespace aurora {

static const Scalar gravity = 50; // m.s-2
static const Scalar KineticCoef = 0.001;

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


static Scalar GasConstant = 1e-5;

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
    Quantity N = Quantity(pressure * volume / (GasConstant * temperature));
    return N;
}


Material PhysicalConstants::GetDissolvedMaterial(Material material)
{
    return dissolvedMaterial[material];
}


////////////////////////
/// FluidNode
////////////////////////

FluidNode::~FluidNode()
{

}

void FluidNode::AddTransition(TransitionLink const& transitionLink)
{
    m_transitionLinks.push_back(transitionLink);
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

void GasNode::PrepareTransitions()
{
    MigrateKineticEnergy();
}

void GasNode::MigrateKineticEnergy()
{
    Energy energyByDirection[4];
    Scalar sectionSumByDirection[4];

    for(int i = 0; i < 4 ; i++)
    {
        energyByDirection[i] = 0;
        sectionSumByDirection[i] = 0;
    }

  auto ComputeEnergy = [&]()
    {
        Energy energy = m_thermalEnergy;
        for(TransitionLink& transitionLink : m_transitionLinks)
        {
            Transition::NodeLink* link = transitionLink.transition->GetNodeLink(transitionLink.index);
            energy += link->outputKineticEnergy;
            energy += link->inputKineticEnergy;
        }
        for(int i = 0; i < 4 ; i++)
        {
            energy += energyByDirection[i];
        }

        return energy;
    };

    Energy initialEnergyCheck = ComputeEnergy();


    Scalar sectionSum = 0;
    
    auto TakeEnergy = [&energyByDirection](Transition::Direction direction, Scalar ratio) 
    {
        Energy takenEnergy  = Energy(energyByDirection[direction] * ratio);
        energyByDirection[direction] -= takenEnergy;
        return takenEnergy;
    };

    const float DiffusionFactor = 1.0; // TODO remove if 1

    for(TransitionLink& transitionLink : m_transitionLinks)
    {

        Transition::NodeLink* link = transitionLink.transition->GetNodeLink(transitionLink.index);

        Transition::Direction linkDirection = transitionLink.transition->GetDirection(transitionLink.index);
        
        Energy energyToDispatch = link->outputKineticEnergy;

        Energy forwardEnergy = energyToDispatch * DiffusionFactor;
        energyToDispatch -= forwardEnergy;

        Energy leftEnergy = energyToDispatch / 2;
        energyToDispatch -= leftEnergy;

        energyByDirection[linkDirection.Opposite()] += forwardEnergy;
        energyByDirection[linkDirection.Next()] += leftEnergy;
        energyByDirection[linkDirection.Previous()] += energyToDispatch;
        link->outputKineticEnergy = 0;
        
        sectionSumByDirection[linkDirection] += transitionLink.transition->GetSection();
        sectionSum += transitionLink.transition->GetSection();
    }

    // If not section in front, divert to side
    for(int i = 0; i < 4 ; i++)
    {
        if(sectionSumByDirection[i] == 0 && energyByDirection[i] > 0)
        {
            uint8_t leftIndex = i+1;
            uint8_t rightIndex = (i+3) & 3;

            Energy leftEnergy = energyByDirection[i] / 2;
            energyByDirection[i] -= leftEnergy;
            energyByDirection[leftIndex] += leftEnergy;
            energyByDirection[rightIndex] += energyByDirection[i];
            energyByDirection[i] = 0;
        }
    }


    // Transform opposite direction as heat or reflect in opposite directions

    const float DiversionRatio = 1.0; // TODO remove if 0

    Energy divertedEnergy[2];

    for(int i = 0; i < 2; i++)
    {
        Energy energyDiff = energyByDirection[i] - energyByDirection[i+2];
        divertedEnergy[i] = energyByDirection[i] + energyByDirection[i+2] - std::abs(energyDiff);
        if(energyDiff > 0)
        {
            energyByDirection[i] = energyDiff;
            energyByDirection[i + 2] = 0;
        }
        else if(energyDiff < 0)
        {
            energyByDirection[i] = 0;
            energyByDirection[i + 2] = -energyDiff;
        }
        else
        {
            energyByDirection[i] = 0;
            energyByDirection[i + 2] = 0;
        }
        
        

        {
            //Energy finalEnergyCheck = ComputeEnergy();
            //assert(finalEnergyCheck == initialEnergyCheck);
        }
    }

    for(int i = 0; i < 2; i++)
    {
        assert(energyByDirection[i] * energyByDirection[i+2] == 0);
    }

    for(int i = 0; i < 2; i++)
    {
        Energy thermalLoss = divertedEnergy[i] * (1-DiversionRatio);
        m_thermalEnergy += thermalLoss;
        divertedEnergy[i] -= thermalLoss;

        uint8_t leftIndex = i+1;
        uint8_t rightIndex = (i+3) & 3;

        if(energyByDirection[leftIndex] == 0 && energyByDirection[rightIndex] == 0)
        {
            // No flow, divert in both direction
            Energy leftEnergy = divertedEnergy[i] / 2;
            divertedEnergy[i] -= leftEnergy;
            energyByDirection[leftIndex] += leftEnergy;
            energyByDirection[rightIndex] += divertedEnergy[i];
        }
        else if (energyByDirection[leftIndex] != 0)
        {
            // Left have flow, use it
            energyByDirection[leftIndex] += divertedEnergy[i];
        }
        else
        {
            assert(energyByDirection[rightIndex] != 0);
            energyByDirection[rightIndex] += divertedEnergy[i];
        }        
    }

    for(int i = 0; i < 2; i++)
    {
       // assert(energyByDirection[i] * energyByDirection[i+2] == 0);
    }
    

    for(TransitionLink& transitionLink : m_transitionLinks)
    {

        Transition::NodeLink* link = transitionLink.transition->GetNodeLink(transitionLink.index);
        Transition::Direction linkDirection = transitionLink.transition->GetDirection(transitionLink.index);

        Scalar section = transitionLink.transition->GetSection();
        Scalar sectionRatio = section / sectionSumByDirection[linkDirection];

        link->inputKineticEnergy += TakeEnergy(linkDirection , sectionRatio);

        sectionSumByDirection[linkDirection] -= section;

        assert(link->inputKineticEnergy  >= 0);
    }

    // Find remaining ene

/*
    for(int i = 0; i < 4 ; i++)
    {
        if(energyByDirection[i] > 0)
        {
            // Rebound or absorb
            // For now rebound to all transitions
            Scalar localSectionSum = sectionSum;

            for(TransitionLink& transitionLink : m_transitionLinks)
            {
                Transition::NodeLink* link = transitionLink.transition->GetNodeLink(transitionLink.index);

                Scalar section = transitionLink.transition->GetSection();
                Scalar sectionRatio = section / localSectionSum;

                link->inputKineticEnergy += TakeEnergy(Transition::Direction(i) , sectionRatio);

                localSectionSum -= section;
                assert(link->inputKineticEnergy  >= 0);
            }
        }
    }*/

        for(int i = 0; i < 4 ; i++)
    {
        if(energyByDirection[i] > 0)
        {
            m_thermalEnergy += TakeEnergy(Transition::Direction(i) , 1.0f);
        }
    }

    
    for(int i = 0; i < 4 ; i++)
    {
        assert(energyByDirection[i] == 0);
    }

    {
        //Energy finalEnergyCheck = ComputeEnergy();
        //assert(finalEnergyCheck == initialEnergyCheck);
    }
}

void GasNode::ApplyTransitions()
{
    // Apply transition output
    for(TransitionLink& transitionLink : m_transitionLinks)
    {
        Transition::NodeLink* link = transitionLink.transition->GetNodeLink(transitionLink.index);
        assert(link->node == this);

        m_thermalEnergy += link->outputThermalEnergy;
        link->outputThermalEnergy = 0;

        for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
        {
            m_nMaterials[gazIndex] += link->outputMaterial[gazIndex];
            link->outputMaterial[gazIndex] = 0;
        }
    }

    //MigrateKineticEnergy();
}

void GasNode::ComputeCache()
{
    // Compute N, energy and mass
    m_cacheCheckN = 0;
    m_cacheMass = 0;
    Energy energyPerK = 0;
    for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
    {
        Quantity materialN = m_nMaterials[gazIndex];

        m_cacheCheckN += materialN;
        if(materialN > 0)
        {
            energyPerK += specifHeat[gazIndex] * materialN;
            m_cacheMass += massByN[gazIndex] * materialN;
        }
    }

    if(m_cacheCheckN < 0)
    {
        m_cacheN = 0;
    }
    else
    {
        m_cacheN = m_cacheCheckN;
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

    if(m_cacheTemperature < 0)
    {
        m_cacheTemperature = 0;
    }

    // Compute pressure
    m_cachePressure = m_cacheN * GasConstant * m_cacheTemperature / m_volume;
    if(m_cachePressure < 0)
    {
        m_cachePressure = 0;
    }

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

Energy GasNode::GetEnergy() const
{
    if(m_thermalEnergy > 0)
    {
        return m_thermalEnergy;
    }
    else
    {
        return 0;
    }
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

///////////////


GasGasTransition::GasGasTransition(GasGasTransition::Config const& config)
    : Transition (config.direction, config.section)
    , m_links {&config.A, &config.B }
    , m_frictionCoef(0.9) // TODO
{
    m_links[0].altitudeRelativeToNode = config.relativeAltitudeA;
    m_links[1].altitudeRelativeToNode = config.relativeAltitudeB;
    m_links[0].longitudeRelativeToNode = config.relativeLongitudeA;
    m_links[1].longitudeRelativeToNode = config.relativeLongitudeB;

    for(int i = 0; i < LinkCount; i++)
    {
        m_links[i].outputThermalEnergy = 0;
        m_links[i].inputKineticEnergy = 0;
        m_links[i].outputKineticEnergy = 0;

        for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
        {
            m_links[i].outputMaterial[gazIndex] = 0;
        }

    }
}

void GasGasTransition::Step(Scalar delta)
{
    //TODO dt

    // For each tile
    // Propagate Ec to from A Ec
    
    // For each transition
    // add all Ec to current Ec
    // compute delta pressure and pressure dp
    // compute dp from Ec
    // compute total dp before friction
    // compute velocity square
    // compute friction lost
    // compute final need dp
    // commit dp, energy ejection


    // For each tile
    // add N, add E lost or gain




    //while negative N or E ask to all transitions some maters and energy
    // add negative tile to a liste and propagate until all tiles are ok



    // TODO no sqrt ?

    // Get energy propagation

    NodeLink& linkA = m_links[0];
    NodeLink& linkB = m_links[1];
    GasNode& A = *((GasNode*) linkA.node);
    GasNode& B = *((GasNode*) linkB.node);


    Energy initialKineticEnergyDelta = linkA.inputKineticEnergy - linkB.inputKineticEnergy; 

    assert(LinkCount == 2);
    Energy kineticEnergySum = 0; 

    for(int i = 0; i < LinkCount; i++)
    {
        assert(m_links[i].inputKineticEnergy >= 0);
        kineticEnergySum += m_links[i].inputKineticEnergy;
        m_links[i].inputKineticEnergy = 0;
        assert(m_links[i].outputThermalEnergy == 0);
        assert(m_links[i].outputKineticEnergy == 0);
    }

    Energy initialCheckEnergy = kineticEnergySum;

    Energy kineticEnergyDelta = initialKineticEnergyDelta;

    if(B.GetTemperature() > 150 || A.GetTemperature() > 150)
    {
        char* plop;
        plop = "hot\n";
    }

    Scalar pressureA = A.GetPressure() + A.GetPressureGradient() * linkA.altitudeRelativeToNode;
    Scalar pressureB = B.GetPressure() + B.GetPressureGradient() * linkB.altitudeRelativeToNode;

    Scalar viscosity = 0.1;

    //Scalar pressureADeltaN = pressureA * m_section * viscosity;
    //Scalar pressureBDeltaN = pressureB * m_section * viscosity;

    //Quantity transfertN = MAX(0, abs(finalDeltaN));


    float diffusionRatio = 0;

    for(int sourceIndex = 0; sourceIndex < LinkCount; sourceIndex++)
    {
        NodeLink& link = m_links[sourceIndex];
        GasNode& sourceNode = *((GasNode*) link.node);
        int destinationIndex = 1 -sourceIndex;

        Scalar pressure = sourceNode.GetPressure() + sourceNode.GetPressureGradient() * link.altitudeRelativeToNode;
        Scalar pressureDeltaN = diffusionRatio * pressure * m_section * viscosity / sourceNode.GetTemperature();

        Quantity sourceTotalN = sourceNode.GetN();
        Quantity transfertN = Quantity(pressureDeltaN);


        if(transfertN > 0)
        {
            for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
            {
                // TODO diffusion
                Scalar compositionRatio = Scalar(sourceNode.GetN(Material(gazIndex))) / Scalar(sourceTotalN);
                Quantity quantity = Quantity(transfertN * compositionRatio);
                m_links[sourceIndex].outputMaterial[gazIndex] -= quantity;
                m_links[destinationIndex].outputMaterial[gazIndex] += quantity;
            }

            // Take energy ratio
            Scalar takenRatio = Scalar(transfertN) / sourceTotalN;
            Energy takenEnergy = Energy(takenRatio * sourceNode.GetEnergy());
            m_links[sourceIndex].outputThermalEnergy -= takenEnergy;
            m_links[destinationIndex].outputThermalEnergy += takenEnergy;
        }
    }

   /* size_t sourceIndex;
    size_t destinationIndex;
    if(pressureA > pressureB)
    {
        sourceIndex = 0;
        destinationIndex = 1;
    }
    else {
        sourceIndex = 1;
        destinationIndex = 0;
    }*/

    //GasNode& sourceNode = *((GasNode*) m_links[sourceIndex].node);

    //Quantity sourceTotalN = sourceNode.GetN();
    //Quantity transfertN = 0;
    //for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
    //{
    //    transfertN += -m_links[sourceIndex].outputMaterial[gazIndex];
    //}

    // Take energy ratio
    //Scalar takenRatio = Scalar(transfertN) / sourceTotalN;
    //Energy takenEnergy = Energy(takenRatio * sourceNode.GetEnergy());
    //m_links[sourceIndex].outputEnergy -= takenEnergy;
    //m_links[destinationIndex].outputEnergy += takenEnergy;

    // Take energy from the source

    // TODO kinetic energy
 



    //Scalar kineticEnergyNeeds = totalDeltaNBeforeFriction * KineticCoef * meanMassMolar / m_section;
    
    
     //Scalar pressure = sourceNode.GetPressure() + sourceNode.GetPressureGradient() * link.altitudeRelativeToNode;
       // Scalar pressureDeltaN = pressure * m_section * viscosity / sourceNode.GetTemperature();


/*
    Scalar pressureDiff = pressureA - pressureB;
    Scalar pressureDeltaN = pressureDiff * m_section * viscosity ;

    // v = dN / area
    // Ec = v * KineticCoef * meanMassMolar
    // dN = v * area
    // Ec = (dN / area) * KineticCoef * meanMassMolar
    // v= Ec / (KineticCoef * meanMassMolar)
    // dN = area * Ec / (KineticCoef * meanMassMolar)



    // TODO meanMassMolar
    Scalar meanMassMolar = 1;




    Scalar kineticDeltaN = m_section * m_kineticEnergy / (KineticCoef * meanMassMolar);
    Scalar totalDeltaNBeforeFriction = pressureDeltaN + kineticDeltaN;

    // TODO diffusion part
    Scalar diffusionRatio = 0;
    if(pressureDeltaN * totalDeltaNBeforeFriction > 0)
    {
        diffusionRatio = MIN(1.f, pressureDeltaN / totalDeltaNBeforeFriction);
    }

    Scalar kineticEnergyNeeds = totalDeltaNBeforeFriction * KineticCoef * meanMassMolar / m_section;
    Energy kineticEnergyNeedsAfterFriction = Energy(kineticEnergyNeeds * m_frictionCoef);

    Energy kineticEnergyDelta = abs(kineticEnergyNeedsAfterFriction) - abs(m_kineticEnergy);

    linkA.outputEnergy = 0;
    linkB.outputEnergy = 0;

    if(kineticEnergyNeedsAfterFriction > 0)
    {
        linkA.outputEnergy = -kineticEnergyDelta;
    }
    else if(kineticEnergyNeedsAfterFriction < 0)
    {
        linkB.outputEnergy = -kineticEnergyDelta;
    }
    else
    {
        if(m_kineticEnergy > 0)
        {
            linkB.outputEnergy = -kineticEnergyDelta;
        }
        else {
            linkA.outputEnergy = -kineticEnergyDelta;
        }
    }

    m_kineticEnergy = kineticEnergyNeedsAfterFriction;

    Quantity finalDeltaN =  Quantity(m_section * m_kineticEnergy / (KineticCoef * meanMassMolar));


    size_t sourceIndex;
    size_t destinationIndex;

    if(finalDeltaN > 0)
    {
        sourceIndex = 0;
        destinationIndex = 1;
    }
    else {
        sourceIndex = 1;
        destinationIndex = 0;
    }





    // TODO gravity energy


    Quantity sourceTotalN = sourceNode.GetN();
    Quantity transfertN = MAX(0, abs(finalDeltaN));

    if(transfertN > 0)
    {
        for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
        {
            // TODO diffusion
            Scalar compositionRatio = Scalar(sourceNode.GetN(Material(gazIndex))) / Scalar(sourceTotalN);
            Quantity quantity = Quantity(transfertN * compositionRatio);
            m_links[sourceIndex].outputMaterial[gazIndex] = -quantity;
            m_links[destinationIndex].outputMaterial[gazIndex] = quantity;

            // TODO take energy !
        }

        // Take energy ratio
        Scalar takenRatio = Scalar(transfertN) / sourceTotalN;
        Energy takenEnergy = Energy(takenRatio * sourceNode.GetEnergy());
        m_links[sourceIndex].outputEnergy -= takenEnergy;
        m_links[destinationIndex].outputEnergy += takenEnergy;
    }*/

    // Apply pressure acceleration
    Scalar pressureDiff = pressureA - pressureB;
    Scalar kineticAcceleration = pressureDiff * m_section * viscosity * KineticCoef;
    Energy newKineticEnergyDelta = kineticEnergyDelta + kineticAcceleration;

    size_t sourceIndex;
    size_t destinationIndex;
    if(newKineticEnergyDelta > 0)
    {
        sourceIndex = 0;
        destinationIndex = 1;
    }
    else {
        sourceIndex = 1;
        destinationIndex = 0;
    }

    

    

    Energy kineticEnergy = abs(newKineticEnergyDelta);

    if(kineticEnergy > 0)
    {
        GasNode& sourceNode = *((GasNode*) m_links[sourceIndex].node);

        Scalar pressureDeltaN = kineticEnergy / (KineticCoef * sourceNode.GetTemperature());

        Quantity sourceTotalN = sourceNode.GetN();
        Quantity transfertN = Quantity(pressureDeltaN);


        if(transfertN > 0)
        {
            for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
            {
                // No diffusion
                Scalar compositionRatio = Scalar(sourceNode.GetN(Material(gazIndex))) / Scalar(sourceTotalN);
                Quantity quantity = Quantity(transfertN * compositionRatio);
                m_links[sourceIndex].outputMaterial[gazIndex] -= quantity;
                m_links[destinationIndex].outputMaterial[gazIndex] += quantity;
            }

            // Take energy ratio
            Scalar takenRatio = Scalar(transfertN) / sourceTotalN;
            Energy takenEnergy = Energy(takenRatio * sourceNode.GetEnergy());
            m_links[sourceIndex].outputThermalEnergy -= takenEnergy;
            m_links[destinationIndex].outputThermalEnergy += takenEnergy;
        }
    }

    assert(m_links[sourceIndex].outputKineticEnergy == 0);
    
    /*// Apply pressure acceleration
    Scalar pressureDiff = pressureA - pressureB;
    Scalar kineticAcceleration = pressureDiff * m_section * viscosity * KineticCoef;
    Energy newKineticEnergyDelta = kineticEnergyDelta + kineticAcceleration;*/
    size_t newSourceIndex;
    size_t newDestinationIndex;
    //TODO energy dissipation

    if(newKineticEnergyDelta > 0)
    {
        newSourceIndex = 0;
        newDestinationIndex = 1;
    }
    else {
        newSourceIndex = 1;
        newDestinationIndex = 0;
    }

    Energy newKineticEnergyBeforeLoss = abs(newKineticEnergyDelta);
    Energy thermalLoss = newKineticEnergyBeforeLoss * 0.08;
    Energy newKineticEnergy = newKineticEnergyBeforeLoss - thermalLoss;


    Energy energyDiff = newKineticEnergy - kineticEnergySum;

    m_links[newDestinationIndex].outputKineticEnergy = newKineticEnergy;

    Energy sourceEnergyDiff =  energyDiff /2;
    Energy destinationEnergyDiff =  energyDiff - sourceEnergyDiff;

    /*if(energyDiff < 0)
    {
        m_links[newSourceIndex].outputThermalEnergy -= energyDiff;
    }
    else
    {
        m_links[newDestinationIndex].outputThermalEnergy -= energyDiff;
    }*/

    m_links[newSourceIndex].outputThermalEnergy -= sourceEnergyDiff;
    m_links[newDestinationIndex].outputThermalEnergy -= destinationEnergyDiff;

/*
    if(energyDiff > 0)
    {
        m_links[newSourceIndex].outputThermalEnergy -= energyDiff;
    }
    else
    {
        m_links[newDestinationIndex].outputThermalEnergy -= energyDiff;
    }*/


    Energy finalCheckEnergy = 0;
    for(int i = 0; i < LinkCount; i++)
    {
        finalCheckEnergy += m_links[i].outputThermalEnergy;
        finalCheckEnergy += m_links[i].outputKineticEnergy;
        assert(m_links[i].inputKineticEnergy == 0);
    }

    assert(initialCheckEnergy == finalCheckEnergy);
}


/////////////////////////

PhysicEngine::~PhysicEngine()
{
    for(Transition* transition : m_transitions)
    {
        delete transition;
    }

    m_transitions.clear();
}


void PhysicEngine::Flush()
{
    m_nodes.clear();

    if(!m_transitions.empty())
    {
        assert(false);
    }
}




void PhysicEngine::Step(Scalar delta)
{
    auto ComputeEnergy = [this]()
    {
        __int128 energy = 0;
        for(FluidNode* node : m_nodes)
        {
            energy += node->GetCheckEnergy();
        }

        for(Transition* transition : m_transitions)
        {
            for(size_t i = 0; i < transition->GetNodeLinkCount(); i++)
            {
                assert(transition->GetNodeLink(i)->inputKineticEnergy >= 0);

                energy += transition->GetNodeLink(i)->inputKineticEnergy;
                energy += transition->GetNodeLink(i)->outputThermalEnergy;
                energy += transition->GetNodeLink(i)->outputKineticEnergy;
            }
        }

        return energy;
    };

    // Check
    __int128 initialTotalEnergy = ComputeEnergy();
    __int128 initialTotalN = 0;
    for(FluidNode* node : m_nodes)
    {
       // initialTotalEnergy += node->GetCheckEnergy();
        initialTotalN += node->GetCheckN();
    }

    /*for(Transition* transition : m_transitions)
    {
        for(size_t i = 0; i < transition->GetNodeLinkCount(); i++)
        {
            assert(transition->GetNodeLink(i)->inputKineticEnergy >= 0);
        }
    }*/

    for(FluidNode* node : m_nodes)
    {
        node->PrepareTransitions();
        // TODO move to post step to remove one compute cache ?
        node->ComputeCache();

        //__int128 check = ComputeEnergy();
        
        //assert(initialTotalEnergy == check);
    }

    /*for(Transition* transition : m_transitions)
    {
        for(size_t i = 0; i < transition->GetNodeLinkCount(); i++)
        {
            assert(transition->GetNodeLink(i)->inputKineticEnergy >= 0);
        }
    }*/

    for(Transition* transition : m_transitions)
    {
        //__int128 check1 = ComputeEnergy();
        //assert(initialTotalEnergy == check1);
        transition->Step(delta);
        //__int128 check2 = ComputeEnergy();
        //assert(initialTotalEnergy == check2);
    }

    __int128 finalTotalEnergy = 0;
        __int128 finalTotalEnergyP1 = 0;
                __int128 finalTotalEnergyP2 = 0;
                __int128 finalTotalEnergyP3 = 0;


    __int128 finalTotalN = 0;
    for(FluidNode* node : m_nodes)
    {
        finalTotalN += node->GetCheckN();
    }

    __int128 check = ComputeEnergy();
// TODO REPAIR
    assert(initialTotalEnergy == check);
    assert(initialTotalN == finalTotalN);
#if 0
    for(FluidNode* node : m_nodes)
    {
        GasNode* gasNode = (GasNode*) node;
        // Artificial radiation cooling
        gasNode->TakeThermalEnergy(0.0001 * gasNode->GetThermalEnergy());



        node->ApplyTransitions();
        node->ComputeCache();
    }
#endif

#if 1
    {
        GasNode* node =(GasNode*) m_nodes[597];
        //node->AddThermalEnergy(10000 * node->GetN());
        node->AddThermalEnergy(0.1 * node->GetN());
        node->ComputeCache();
        printf("N=%lld\n",node->GetN());
    }
#endif

#if 1
    {
        GasNode* node =(GasNode*) m_nodes[123];
        //node->AddThermalEnergy(10000 * node->GetN());
        
        //node->AddThermalEnergy(-0.1 * node->GetN());
        node->TakeThermalEnergy(0.1 * node->GetThermalEnergy());
        node->ComputeCache();
        printf("N=%lld\n",node->GetN());
    }
#endif
}

void PhysicEngine::CheckDuplicates()
{
    for(size_t i = 0 ; i< m_transitions.size(); i++)
    {
        for(size_t j = i+1 ; j< m_transitions.size(); j++)
        {
            assert(m_transitions[i] != m_transitions[j]);
            assert(!(m_transitions[i]->GetNodeA() == m_transitions[j]->GetNodeA() && m_transitions[i]->GetNodeB() == m_transitions[j]->GetNodeB()));
            assert(!(m_transitions[i]->GetNodeA() == m_transitions[j]->GetNodeB() && m_transitions[i]->GetNodeB() == m_transitions[j]->GetNodeA()));
        }
    }
}

void PhysicEngine::AddTransition(Transition* transition)
{
    m_transitions.push_back(transition);
    transition->GetNodeA()->AddTransition(TransitionLink(transition, 0));
    transition->GetNodeB()->AddTransition(TransitionLink(transition, 1));
}

void PhysicEngine::AddFluidNode(FluidNode* node)
{
    m_nodes.push_back(node);
}

Transition::Direction Transition::GetDirection(size_t index) const
{
    if(index == 0)
    {
        return m_direction;
    }
    else
    {
        return m_direction.Opposite();
    }
}




}

