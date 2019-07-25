#ifndef AURORA_WORLD_EDITOR_H
#define AURORA_WORLD_EDITOR_H

#include "aurora_world.h"
#include "aurora_tile.h"
#include "../physics/aurora_physic_engine.h"

namespace aurora {

struct TileGasComposition
{
    TileGasComposition();

    Quantity composition[Material::GasMoleculeCount];
    Scalar pressure;
    Scalar temperature;
};

struct TileLiquidVolume
{
    Material material;
    Scalar volumeProportion;
    Scalar dissolvedProportion;
    Scalar temperature;
    Scalar pressure;
};

struct TileSolidVolume
{
    Material material;
    Quantity volumePart;
};

struct TileComposition
{
    TileGasComposition Gas;
    std::vector<TileLiquidVolume> liquids;
    Scalar porosity; // Quantity of volume not used by solid, if solid present
    std::vector<TileSolidVolume> solids;
    Scalar solidTemperature;

    void AddLiquidVolume(Material material, Scalar volumeProportion, Scalar dissolvedProportion, Scalar temperature, Scalar pressure);
    void AddSolidVolume(Material material, Volume volumePart);
};


class WorldEditor{
public:
    WorldEditor(AuroraWorld& world);

    virtual ~WorldEditor();

    void GenerateTestWord();
    void GenerateHelloWord();

private:
    AuroraWorld& m_world;

    void PaintTiles(Level* level, TileComposition const& composition, MmRect area);
    void PaintTile(Level* level, Tile* tile, TileComposition const& composition, MmRect area);
    void SetTileComposition(Tile* tile, TileComposition composition);

    void Repack(Level* level);
};

}

#endif
