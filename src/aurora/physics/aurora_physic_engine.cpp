#include "aurora_physic_engine.h"

#include <assert.h>

namespace aurora {

static const Scalar gravity = 100; // m.s-2
static const Scalar KineticCoef = 100;

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
    // Migrate kinetic energy
    // Fill transition input
    // TODO
}

void GasNode::ApplyTransitions()
{
    // Apply transition output
    for(TransitionLink& transitionLink : m_transitionLinks)
    {
        Transition::NodeLink* link = transitionLink.transition->GetNodeLink(transitionLink.index);
        assert(link->node == this);


        m_thermalEnergy += link->outputEnergy;
        link->outputEnergy = 0;

        for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
        {
            m_nMaterials[gazIndex] += link->outputMaterial[gazIndex];
            link->outputMaterial[gazIndex] = 0;
        }
    }
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
        m_cacheTemperature = m_cacheTemperature;
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
    : Transition (config.direction)
    , m_links {&config.A, &config.B }
    , m_frictionCoef(0.9) // TODO
    , m_section(config.section)
    , m_kineticEnergy(0)
{
    m_links[0].altitudeRelativeToNode = config.relativeAltitudeA;
    m_links[1].altitudeRelativeToNode = config.relativeAltitudeB;

    for(int i = 0; i < LinkCount; i++)
    {
        m_links[i].inputEnergy = 0;
        m_links[i].outputEnergy = 0;

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
    for(int i = 0; i < LinkCount; i++)
    {
        m_kineticEnergy += m_links[i].inputEnergy;
        m_links[i].inputEnergy = 0;
    }


    Energy initialCheckEnergy = abs(m_kineticEnergy);
    for(int i = 0; i < LinkCount; i++)
    {
        assert(m_links[i].outputEnergy == 0);
    }

    NodeLink& linkA = m_links[0];
    NodeLink& linkB = m_links[1];
    GasNode& A = *((GasNode*) linkA.node);
    GasNode& B = *((GasNode*) linkB.node);


    if(B.GetTemperature() > 200)
    {
        char* plop;
        plop = "hot\n";
    }

    Scalar pressureA = A.GetPressure() + A.GetPressureGradient() * linkA.altitudeRelativeToNode;
    Scalar pressureB = B.GetPressure() + B.GetPressureGradient() * linkB.altitudeRelativeToNode;

    Scalar viscosity = 0.001;

    //Scalar pressureADeltaN = pressureA * m_section * viscosity;
    //Scalar pressureBDeltaN = pressureB * m_section * viscosity;

    //Quantity transfertN = MAX(0, abs(finalDeltaN));



    for(int sourceIndex = 0; sourceIndex < LinkCount; sourceIndex++)
    {
        NodeLink& link = m_links[sourceIndex];
        GasNode& sourceNode = *((GasNode*) link.node);
        int destinationIndex = 1 -sourceIndex;

        Scalar pressure = sourceNode.GetPressure() + sourceNode.GetPressureGradient() * link.altitudeRelativeToNode;
        Scalar pressureDeltaN = pressure * m_section * viscosity;

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
            m_links[sourceIndex].outputEnergy -= takenEnergy;
            m_links[destinationIndex].outputEnergy += takenEnergy;
        }
    }

    size_t sourceIndex;
    size_t destinationIndex;
    if(pressureA > pressureB)
    {
        sourceIndex = 0;
        destinationIndex = 1;
    }
    else {
        sourceIndex = 1;
        destinationIndex = 0;
    }

    GasNode& sourceNode = *((GasNode*) m_links[sourceIndex].node);

    Quantity sourceTotalN = sourceNode.GetN();
    Quantity transfertN = 0;
    for(int gazIndex = 0; gazIndex < Material::GasMoleculeCount; gazIndex++)
    {
        transfertN += -m_links[sourceIndex].outputMaterial[gazIndex];
    }

    // Take energy ratio
    Scalar takenRatio = Scalar(transfertN) / sourceTotalN;
    Energy takenEnergy = Energy(takenRatio * sourceNode.GetEnergy());
    //m_links[sourceIndex].outputEnergy -= takenEnergy;
    //m_links[destinationIndex].outputEnergy += takenEnergy;

    // Take energy from the source

    // TODO kinetic energy


    /*Scalar pressureDiff = pressureA - pressureB;
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

    GasNode& sourceNode = *((GasNode*) m_links[sourceIndex].node);

    Quantity sourceTotalN = sourceNode.GetN();
    Quantity transfertN = MAX(0, abs(finalDeltaN));*/

    /*if(transfertN > 0)
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

    Energy finalCheckEnergy = abs(m_kineticEnergy);
    for(int i = 0; i < LinkCount; i++)
    {
        finalCheckEnergy += m_links[i].outputEnergy;
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
    // Check
    __int128 initialTotalEnergy = 0;
    __int128 initialTotalN = 0;
    for(FluidNode* node : m_nodes)
    {
        initialTotalEnergy += node->GetCheckEnergy();
        initialTotalN += node->GetCheckN();
    }

    for(Transition* transition : m_transitions)
    {
        initialTotalEnergy += abs(transition->GetEnergy());
    }


    for(FluidNode* node : m_nodes)
    {
        node->PrepareTransitions();
    }

    for(Transition* transition : m_transitions)
    {
        transition->Step(delta);
    }

    for(FluidNode* node : m_nodes)
    {
        node->ApplyTransitions();
        node->ComputeCache();
    }


    __int128 finalTotalEnergy = 0;
    __int128 finalTotalN = 0;
    for(FluidNode* node : m_nodes)
    {
        finalTotalEnergy += node->GetCheckEnergy();
        finalTotalN += node->GetCheckN();
    }

    for(Transition* transition : m_transitions)
    {
        finalTotalEnergy += abs(transition->GetEnergy());
    }

    assert(initialTotalEnergy == finalTotalEnergy);
    assert(initialTotalN == finalTotalN);

    GasNode* node =(GasNode*) m_nodes[700];
    //node->AddThermalEnergy(10000);
    node->ComputeCache();
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


}

