#ifndef AURORA_PHYSIC_ENGINE_H
#define AURORA_PHYSIC_ENGINE_H

#include "core/math/vector2.h"
#include "core/math/rect2.h"
#include "../tools/aurora_types.h"
#include <vector>

//class AuroraTile;

namespace aurora {
/*
enum GasMaterial
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
};*/

enum Material
{
    Oxygen, // O2
    Nitrogen, // N2
    Water,  // H2O
    Methane,// CH4
    CarbonDioxide, // CO2


    //

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

class PhysicalConstants
{
public:
    static Quantity GetSolidNByVolume(Material material, Volume volume);

    static Volume GetSolidVolumeByN(Material material, Quantity N);

    static Quantity EstimateLiquidNByVolume(Material material, Volume volume, Scalar pressure, Scalar temperature);

    static Volume GetLiquidVolumeByN(Material material, Quantity N, Scalar pressure, Scalar temperature);

    static Energy EstimateThermalEnergy(Material material, Quantity N, Scalar temperature);

    static Quantity EstimateGasN(Volume volume, Scalar pressure, Scalar temperature);


    static Material GetDissolvedMaterial(Material material);
};

class GasNode
{
public:
    GasNode();
    GasNode(GasNode& node);

    Volume GetVolume() const;
    void SetVolume(Volume volume);

    Quantity GetN(Material material) const;
    Energy GetThermalEnergy() const;

    void AddN(Material material, Quantity N);
    void AddThermalEnergy(Energy thermalEnergy);



    void TakeN(Material material, Quantity N);
    void TakeThermalEnergy(Energy thermalEnergy);

    Scalar ComputePressure() const;
    Scalar ComputeTemperature() const;
    Quantity ComputeN() const;
    void ComputeNPT(Quantity& N, Scalar& pressure, Scalar& temperature) const;

private:
    // Constants
    Mm m_altitude;
    Volume m_volume;
    //int8_t m_transitionCount;

    // Variables
    //Quantity m_N; // TODO cache ?
    Quantity m_nMaterials[Material::GasMoleculeCount];
    Energy m_thermalEnergy;
};

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

class GasGasTransition
{
    GasGasTransition(GasNode& A, GasNode& B);

    // Contants
    GasNode& m_A;
    GasNode& m_B;
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

class GasLiquidTransition
{
    GasLiquidTransition(GasNode& A, LiquidNode& B);

    // Contants
    GasNode& m_A;
    LiquidNode& m_B;
    Mm m_altitudeA;
    Mm m_altitudeB;
    Scalar m_frictionCoef;

    // Variables
    Energy m_kineticEnergy;
};

class SharedVolumeGasLiquidTransition
{
    SharedVolumeGasLiquidTransition(GasNode& A, LiquidNode& B);

    // Contants
    GasNode& m_A;
    LiquidNode& m_B;
    Mm m_altitudeA;
    Mm m_altitudeB;
    Scalar m_frictionCoef;

    // Variables
    Energy m_kineticEnergy;
};

class PhysicEngine
{


};


}

#endif
