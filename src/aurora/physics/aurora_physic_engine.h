#ifndef AURORA_PHYSIC_ENGINE_H
#define AURORA_PHYSIC_ENGINE_H

#include "core/math/vector2.h"
#include "core/math/rect2.h"
#include "../tools/aurora_types.h"
#include <vector>

//class AuroraTile;

namespace aurora {

enum GazMaterial
{
    Steam, // H2O
    Oxygen, // O2
    Nitrogen, // N2
    CarbonDioxide, // CO2
    UraniumHexafluoride, // UF6
    Methane,// CH4
    Count
};

enum LiquidMaterial
{
    Water,
    MeltedIron,
    Oil,
};


enum SolidMaterial
{
    Ice,
    Iron,
    Salt,
    Sulfur,
    Dirt,
    Limestone, // CaCO3
    Wood,
    Charcoal,
    Coal,
    Coke,
    Gold,
    Clay

};

class GazNode
{
    // Constants
    Unit m_altitude;
    Unit m_volume;
    int8_t m_transitionCount;

    // Variables
    Quantity m_N; // TODO cache ?
    Quantity m_nMaterials[GazMaterial::Count];
    int64_t m_thermalEnergy;
};

class LiquidNode
{
    // Constants
    int32_t m_altitude;
    LiquidMaterial m_material;

    // Variables
    int32_t m_volume;
    int64_t m_N;
    int64_t m_NDiluted;
};

class GazGazTransition
{
    GazGazTransition(GazNode& A, GazNode& B);

    // Contants
    GazNode& m_A;
    GazNode& m_B;
    int32_t m_altitudeA;
    int32_t m_altitudeB;
    float m_frictionCoef;

    // Variables
    int64_t m_kineticEnergy;
};

class LiquidLiquidTransition
{
    LiquidLiquidTransition(LiquidNode& A, LiquidNode& B);

    // Contants
    LiquidNode& m_A;
    LiquidNode& m_B;
    int32_t m_altitudeA;
    int32_t m_altitudeB;
    float m_frictionCoef;

    // Variables
    int64_t m_kineticEnergy;
};

class GazLiquidTransition
{
    GazLiquidTransition(GazNode& A, LiquidNode& B);

    // Contants
    GazNode& m_A;
    LiquidNode& m_B;
    Unit m_altitudeA;
    Unit m_altitudeB;
    Scalar m_frictionCoef;

    // Variables
    energy m_kineticEnergy;
};

class SharedVolumeGazLiquidTransition
{
    SharedVolumeGazLiquidTransition(GazNode& A, LiquidNode& B);

    // Contants
    GazNode& m_A;
    LiquidNode& m_B;
    Unit m_altitudeA;
    Unit m_altitudeB;
    Scalar m_frictionCoef;

    // Variables
    energy m_kineticEnergy;
};

class PhysicEngine
{


};


}

#endif
