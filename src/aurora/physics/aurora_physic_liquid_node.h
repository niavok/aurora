#ifndef AURORA_PHYSIC_LIQUID_NODE_H
#define AURORA_PHYSIC_LIQUID_NODE_H

#include "aurora_physic_types.h"

namespace aurora {


class LiquidNode
{
public:
    LiquidNode(Material material);

    Volume GetVolume() const;
    void SetVolume(Volume volume);

    Material GetMaterial() const;
    Quantity GetN() const;
    Quantity GetDissolvedN() const;
    Energy GetThermalEnergy() const;

    void AddN(Quantity N);
    void AddDissolvedN(Quantity N);
    void AddThermalEnergy(Energy thermalEnergy);

    void TakeN(Quantity N);
    void TakeDissolvedN(Quantity N);
    void TakeThermalEnergy(Energy thermalEnergy);

private:

    // Constants
    Material m_material;
    Mm m_altitude;

    // Variables
    Volume m_volume;
    Quantity m_N;
    Quantity m_dissolvedN;
    Energy m_thermalEnergy;
};

}

#endif
