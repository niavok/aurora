#include "aurora_physic_liquid_node.h"


namespace aurora {

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