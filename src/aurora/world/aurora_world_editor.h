#ifndef AURORA_WORLD_EDITOR_H
#define AURORA_WORLD_EDITOR_H

#include "aurora_world.h"
#include "../physics/aurora_physic_engine.h"

namespace aurora {

struct TileGazComposition
{
   Quantity composition[GazMaterial::Count];
   Scalar pressure;
   Scalar temperature;
};

struct TileLiquidVolume
{
    LiquidMaterial material;
    Quantity quantity;
    Quantity dissolvedQuantity;
    Scalar temperature;
    Scalar pressure;
};

struct TileSolidVolume
{
    SolidMaterial material;
    Quantity quantity;
};

struct TileComposition
{
    TileGazComposition gaz;
    std::vector<TileLiquidVolume> liquids;
    Scalar porosity; // Quantity of volume not used by solid, if solid present
    std::vector<TileSolidVolume> solids;
    Scalar solidTemperature;

    void AddLiquidVolume(LiquidMaterial material, Quantity quantity, Quantity dissolvedQuantity, Scalar temperature, Scalar pressure);
    void AddSolidVolume(SolidMaterial material, Quantity quantity);
};


class WorldEditor{
public:
    WorldEditor(AuroraWorld& world);

    virtual ~WorldEditor();

    void GenerateTestWord();

private:
    AuroraWorld& m_world;

    void PaintTiles(Level* level, TileComposition const& composition, UnitRect area);
    void Repack(Level* level);
};

}

#endif
