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
    Hydrogen, //H2
    Nitrogen, // N2
    Oxygen, // O2
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
    static Quantity EstimateSolidNByVolume(Material material, Volume volume);

    static Volume GetSolidVolumeByN(Material material, Quantity N);

    static Quantity EstimateLiquidNByVolume(Material material, Volume volume, Scalar pressure, Scalar temperature);

    static Volume GetLiquidVolumeByN(Material material, Quantity N, Scalar pressure, Scalar temperature);

    static Energy EstimateThermalEnergy(Material material, Quantity N, Scalar temperature);

    static Quantity EstimateGasN(Volume volume, Scalar pressure, Scalar temperature);


    static Material GetDissolvedMaterial(Material material);
};

class Transition;

struct TransitionLink
{
    TransitionLink(Transition* iTransition, size_t iIndex) : transition(iTransition), index(iIndex) {}

    Transition* transition;
    size_t index;
};

class FluidNode
{
public:

    virtual ~FluidNode();

    virtual void ComputeCache() = 0;
    virtual void PrepareTransitions() = 0;
    virtual void ApplyTransitions() = 0;
    virtual Energy GetEnergy() const = 0;
    virtual Energy GetCheckEnergy() const = 0;
    virtual Quantity GetCheckN() const = 0;

    void AddTransition(TransitionLink const& transitionLink);

protected:
    std::vector<TransitionLink> m_transitionLinks;
};

class GasNode : public FluidNode
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

    Scalar GetPressure() const;
    Scalar GetTemperature() const;
    Quantity GetN() const;
    Scalar GetPressureGradient() const;


    Energy GetEnergy() const;
    Energy GetCheckEnergy() const { return m_thermalEnergy; }
    Quantity GetCheckN() const { return m_cacheCheckN; }

    //Scalar ComputePressure() const;
    //Scalar ComputeTemperature() const;
    //Quantity ComputeN() const;
    //void ComputeNPT(Quantity& N, Scalar& pressure, Scalar& temperature) const;

    void PrepareTransitions();
    void ApplyTransitions();
    void ComputeCache();

private:


    // Constants
    Mm m_altitude;
    Volume m_volume;
    //int8_t m_transitionCount;

    // Variables
    //Quantity m_N; // TODO cache ?
    Quantity m_nMaterials[Material::GasMoleculeCount];
    Energy m_thermalEnergy;

    // Cache
    bool m_cacheComputed;
    Quantity m_cacheN;
    Quantity m_cacheCheckN;
    Scalar m_cacheMass;
    Scalar m_cachePressure;
    Scalar m_cachePressureGradient;
    Scalar m_cacheTemperature;
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

class Transition
{
public:
    enum Direction {
        DIRECTION_UP,
        DIRECTION_RIGHT,
        DIRECTION_DOWN,
        DIRECTION_LEFT,
    };

    Transition(Direction direction) : m_direction(direction) {}

    virtual ~Transition() {}
    virtual void Step(Scalar delta) = 0;

    virtual FluidNode* GetNodeA() = 0;
    virtual FluidNode* GetNodeB() = 0;

    struct NodeLink
    {
        NodeLink(FluidNode* iNode) : node(iNode) {}

        // Contants
        FluidNode* node;
        Meter altitudeRelativeToNode;

        // Input
        Energy inputEnergy;

        // Ouput
        Energy outputEnergy;
        Quantity outputMaterial[Material::GasMoleculeCount];
    };

    virtual NodeLink* GetNodeLink(size_t index) = 0;

    virtual Energy GetEnergy() const = 0;

private:
    Direction m_direction;
};

class GasGasTransition : public Transition
{
public:

    struct Config
    {
        Config(GasNode& iA, GasNode& iB): A(iA), B(iB) { }
        GasNode& A;
        GasNode& B;
        Direction direction;
        Meter relativeAltitudeA;
        Meter relativeAltitudeB;
        Meter section = 0;
    };

    GasGasTransition(Config const& config);

    FluidNode* GetNodeA() { return m_links[0].node; }
    FluidNode* GetNodeB() { return m_links[1].node; }

    NodeLink* GetNodeLink(size_t index) { return &m_links[index]; }

    Energy GetEnergy() const { return m_kineticEnergy; }

    void Step(Scalar delta);

    static const int LinkCount = 2;

private:


    NodeLink m_links[LinkCount];

    // Contants
    //GasNode& m_A;
    //GasNode& m_B;
    //Meter m_altitudeA;
    //Meter m_altitudeB;
    Scalar m_frictionCoef;
    Scalar m_section;

    // Variables
    Energy m_kineticEnergy;
    //Energy m_inputEnergyFromA;
    //Energy m_inputEnergyFromB;

    //Energy m_outputEnergyBalanceToA;
    //Energy m_outputEnergyBalanceToB;
    //Quantity m_outputMaterialBalanceA[Material::GasMoleculeCount];
    //Quantity m_outputMaterialBalanceB[Material::GasMoleculeCount];
};

class LiquidLiquidTransition : public Transition
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

class GasLiquidTransition : public Transition
{
    GasLiquidTransition(GasNode& A, LiquidNode& B, Meter altitudeA, Meter altitudeB);

    // Contants
    GasNode& m_A;
    LiquidNode& m_B;
    Meter m_altitudeA;
    Meter m_altitudeB;
    Scalar m_frictionCoef;

    // Variables
    Energy m_kineticEnergy;
};

class SharedVolumeGasLiquidTransition : public Transition
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
public:
    ~PhysicEngine();

    void Step(Scalar delta);

    void Flush(); // Delete transition but not nodes

    void CheckDuplicates();

    void AddTransition(Transition* transition);

    void AddFluidNode(FluidNode* node);

private:

    std::vector<Transition*> m_transitions;
    std::vector<FluidNode*> m_nodes;
};


}

#endif
