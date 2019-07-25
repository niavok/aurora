#include "aurora_world_editor.h"
#include "aurora_level.h"

namespace aurora {





WorldEditor::WorldEditor(AuroraWorld& world)
    : m_world(world)
{
}

WorldEditor::~WorldEditor()
{
}

void WorldEditor::GenerateHelloWord()
{
    //Level* surfaceLevel = m_world.CreateLevel(50, 8, 1, 1); // 1 x 1 blocks * 50 mm * 2^ 8 = 12.8 m x 12.8 m
    Level* surfaceLevel = m_world.CreateLevel(50, 0, 256, 256); // 256 x 256 blocks * 50 mm * 2^ 1 = 12.8 m x 12.8 m


    TileComposition dryAir;
    // No solid
    // No liquid
    // Nitrogen 80%, Oxygen 20%
    // Temperature 125K
    // Pressure 1 bar
    dryAir.Gas.composition[Material::Nitrogen] = 80;
    dryAir.Gas.composition[Material::Oxygen] = 20;
    dryAir.Gas.pressure = 1.;
    dryAir.Gas.temperature = 125.;



    int surfaceWidth = surfaceLevel->GetSize().x;
    int surfaceHeight = surfaceLevel->GetSize().y;
    PaintTiles(surfaceLevel, dryAir,  MmRect(0,0, surfaceWidth,surfaceHeight));

}


void WorldEditor::GenerateTestWord()
{

    Level* surfaceLevel = m_world.CreateLevel(50, 12, 5, 2); // 20 x 5 blocks = 4096 m x 1024 m

    //cave1 = m_world.CreateLevel(64, 1,1); // 1 x 1 block = 64 m x 64 m


    TileComposition dryAir;
    // No solid
    // No liquid
    // Nitrogen 80%, Oxygen 20%
    // Temperature 125K
    // Pressure 1 bar
    dryAir.Gas.composition[Material::Nitrogen] = 80;
    dryAir.Gas.composition[Material::Oxygen] = 20;
    dryAir.Gas.pressure = 1.f;
    dryAir.Gas.temperature = 125.f;

    TileComposition saltWater;
    // Water 100% of water with 10% of salt, 115 K, at 1 bar
    saltWater.AddLiquidVolume(Material::Water, 1.f, 0.1f , 115, 1);


    TileComposition clayRock;
    clayRock.porosity = 0;
    clayRock.solidTemperature = 120;
    clayRock.AddSolidVolume(Material::Clay, 1000);
    clayRock.AddSolidVolume(Material::Gold, 1);

    int surfaceWidth = surfaceLevel->GetSize().x;
    int surfaceHeight = surfaceLevel->GetSize().y;
    PaintTiles(surfaceLevel, dryAir,  MmRect(0,0, surfaceWidth,surfaceHeight/3));
    PaintTiles(surfaceLevel, clayRock,  MmRect(0, surfaceHeight/3, surfaceWidth, surfaceHeight));

            //PaintTile(Rect2(0,0, worldWidth,worldHeight/2), air);
            //PaintTile(Rect2(0,worldHeight/3, worldWidth,2 * worldHeight/3), limestoneRock);
            //PaintTile(Rect2(worldWidth/2,worldHeight/3 - 10, 10,10), limestoneRock);
            //PaintTile(Rect2(worldWidth/3,worldHeight/3-200 , 100,200), limestoneRock);

    // TODO
    //Repack(surfaceLevel);

/*
    int worldBlockWidth = 16; // 4 km
    int worldBlockHeight = 4; // 1 km

    scalar worldWidth = worldBlockWidth * maxBlockSize;
    scalar worldHeight = worldBlockHeight * maxBlockSize;

    m_worldArea = Rect2(0,0,worldWidth, worldHeight);




    // Fill world

    // Air
    AuroraMaterial air;
    air.AddGasMaterial(MaterialQuantity(Material::Nitrogen, 78, 0));
    air.AddGasMaterial(MaterialQuantity(Material::Oxygen, 21, 0));
    air.AddGasMaterial(MaterialQuantity(Material::Water, 1, 0));
    air.SetTemperature(25);

    AuroraMaterial limestoneRock;
    limestoneRock.SetSolidProperties(false);
    limestoneRock.AddSolidMaterial(MaterialQuantity(Material::Limestone, 0.8, 1));
    limestoneRock.SetTemperature(25);


    PaintTile(Rect2(0,0, worldWidth,worldHeight/2), air);
    PaintTile(Rect2(0,worldHeight/3, worldWidth,2 * worldHeight/3), limestoneRock);
    PaintTile(Rect2(worldWidth/2,worldHeight/3 - 10, 10,10), limestoneRock);
    PaintTile(Rect2(worldWidth/3,worldHeight/3-200 , 100,200), limestoneRock);
*/
    //Repack();
}

void WorldEditor::PaintTiles(Level* level, TileComposition const& composition, MmRect area)
{
    for(Tile* tile : level->GetRootTiles())
    {
        PaintTile(level, tile, composition, area);
    }
}

void WorldEditor::PaintTile(Level* level, Tile* tile, TileComposition const& composition, MmRect area)
{
    //printf("PaintTile area=%ls tile_area=%ls\n", String(area).c_str(), String(m_worldArea).c_str());

    switch(tile->IsInside(area))
    {
    case Tile::InsideMode::Yes:
    {
        //printf("inside: Yes\n");
        // All the tile is in the target area, put the material everywhere
        SetTileComposition(tile, composition);
    }
    break;
    case Tile::InsideMode::Partially:
    {
        //printf("inside: Partially\n");
        if(tile->Split(level))
        {
            //printf("split ok\n");
            for(Tile* child: tile->GetChildren())
            {
                //printf("split child\n");
                PaintTile(level, child, composition, area);
            }
            //printf("split done\n");
        }
        else
        {
            //printf("split ko\n");
            // Cannot split more, already at min size, override
            SetTileComposition(tile, composition);
        }
    }
    break;
    case Tile::InsideMode::No:
        //printf("inside: No\n");
    break;
    }
}

void WorldEditor::SetTileComposition(Tile* tile, TileComposition composition)
{
    Volume volume = tile->GetVolume();

    TileContent newContent(volume);

    if(composition.solids.size() > 0)
    {
        Volume solidsVolume = Volume(volume * (1.f - composition.porosity));

        Volume remainingVolume = solidsVolume;
        Volume remainingVolumePart = 0;

        for (TileSolidVolume& solid : composition.solids) {
            remainingVolumePart += solid.volumePart;
        }

        for (TileSolidVolume& solid : composition.solids) {
            Scalar proportion = Scalar(solid.volumePart) / Scalar(remainingVolumePart);
            Volume solidVolume = Volume(remainingVolume * proportion);

            remainingVolume -= solidVolume;
            remainingVolumePart-= solid.volumePart;

            Quantity solidN = PhysicalConstants::GetSolidNByVolume(solid.material, solidVolume);

            Energy thermalEnergy = PhysicalConstants::EstimateThermalEnergy(solid.material, solidN, composition.solidTemperature);

            newContent.AddSolid(solid.material, solidN, thermalEnergy);
        }
    }

    if(composition.liquids.size() > 0)
    {
        Volume availableVolume = newContent.GetTotalVolume() - newContent.GetSolidsVolume();

         for (TileLiquidVolume& liquid : composition.liquids) {
            Volume liquidVolume = Volume(availableVolume * liquid.volumeProportion);


            Quantity totalQuantity = PhysicalConstants::EstimateLiquidNByVolume(liquid.material, liquidVolume, liquid.pressure, liquid.temperature);
            Quantity dissolvedQuantity = Quantity(totalQuantity * liquid.dissolvedProportion);
            Quantity liquidQuantity = totalQuantity - dissolvedQuantity;

            Energy thermalEnergy = PhysicalConstants::EstimateThermalEnergy(liquid.material, liquidQuantity, liquid.temperature);
            thermalEnergy += PhysicalConstants::EstimateThermalEnergy(PhysicalConstants::GetDissolvedMaterial(liquid.material), dissolvedQuantity, liquid.temperature);

            newContent.AddLiquid(liquid.material, liquidQuantity, dissolvedQuantity, thermalEnergy);
         }
    }

    Quantity GasPartSum = 0;
    for (int GasIndex = 0; GasIndex < Material::GasMoleculeCount; GasIndex++)
    {
        GasPartSum += composition.Gas.composition[GasIndex];
    }

    if(GasPartSum > 0)
    {
        Volume GasVolume = newContent.GetGasVolume();
        Quantity totalN = PhysicalConstants::EstimateGasN(GasVolume, composition.Gas.pressure, composition.Gas.temperature);

        for (int GasIndex = 0; GasIndex < Material::GasMoleculeCount; GasIndex++)
        {
            if(GasPartSum <= 0)
            {
                break;
            }

            Scalar proportion = Scalar(composition.Gas.composition[GasIndex]) / Scalar(GasPartSum);
            Volume GasN= Volume(totalN * proportion);

            totalN -= GasN;
            GasPartSum-= composition.Gas.composition[GasIndex];

            Energy thermalEnergy = PhysicalConstants::EstimateThermalEnergy(Material(GasIndex), GasN, composition.Gas.temperature);

            newContent.AddGas(Material(GasIndex), GasN, thermalEnergy);
        }
    }
    tile->SetContent(newContent);
}

void TileComposition::AddLiquidVolume(Material material, Scalar volumeProportion, Scalar dissolvedProportion, Scalar temperature, Scalar pressure)
{
    TileLiquidVolume volume;
    volume.material = material;
    volume.volumeProportion = volumeProportion;
    volume.dissolvedProportion = dissolvedProportion;
    volume.temperature = temperature;
    volume.pressure = pressure;
     liquids.push_back(volume);
}

void TileComposition::AddSolidVolume(Material material, Volume volumePart)
{
    TileSolidVolume volume;
    volume.material = material;
    volume.volumePart = volumePart;
    solids.push_back(volume);
}

TileGasComposition::TileGasComposition()
{
    for(int i = 0; i < Material::GasMoleculeCount; i++)
    {
        composition[i] = 0;
    }
}



}
