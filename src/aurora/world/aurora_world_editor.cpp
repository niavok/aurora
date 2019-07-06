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

void WorldEditor::GenerateTestWord()
{

    Level* surfaceLevel = m_world.CreateLevel(50, 12, 20, 5); // 20 x 5 blocks = 4096 m x 1024 m

    //cave1 = m_world.CreateLevel(64, 1,1); // 1 x 1 block = 64 m x 64 m


    TileComposition dryAir;
    // No solid
    // No liquid
    // Nitrogen 80%, Oxygen 20%
    // Temperature 125K
    // Pressure 1 bar
    dryAir.gaz.composition[GazMaterial::Nitrogen] = 80;
    dryAir.gaz.composition[GazMaterial::Oxygen] = 20;
    dryAir.gaz.pressure = 1.f;
    dryAir.gaz.temperature = 125.f;

    TileComposition saltWater;
    // Water with 10% of salt, 115 K, at 1 bar
    saltWater.AddLiquidVolume(LiquidMaterial::Water, 100, 10, 115, 1);


    TileComposition clayRock;
    clayRock.porosity = 0;
    clayRock.solidTemperature = 120;
    clayRock.AddSolidVolume(SolidMaterial::Clay, 1000);
    clayRock.AddSolidVolume(SolidMaterial::Gold, 1);

    int surfaceWidth = surfaceLevel->GetSize().x;
    int surfaceHeight = surfaceLevel->GetSize().y;
    PaintTiles(surfaceLevel, dryAir,  UnitRect(0,0, surfaceWidth,surfaceHeight/3));
    PaintTiles(surfaceLevel, clayRock,  UnitRect(0, surfaceHeight/3, surfaceWidth, surfaceHeight));

            //PaintTile(Rect2(0,0, worldWidth,worldHeight/2), air);
            //PaintTile(Rect2(0,worldHeight/3, worldWidth,2 * worldHeight/3), limestoneRock);
            //PaintTile(Rect2(worldWidth/2,worldHeight/3 - 10, 10,10), limestoneRock);
            //PaintTile(Rect2(worldWidth/3,worldHeight/3-200 , 100,200), limestoneRock);

    Repack(surfaceLevel);

/*
    int worldBlockWidth = 16; // 4 km
    int worldBlockHeight = 4; // 1 km

    scalar worldWidth = worldBlockWidth * maxBlockSize;
    scalar worldHeight = worldBlockHeight * maxBlockSize;

    m_worldArea = Rect2(0,0,worldWidth, worldHeight);

    int worldRootTileCount = worldBlockHeight *  worldBlockWidth;

    auto plop = []() {
        printf("plop\n");
    };

    plop();

    m_rootTiles.reserve(worldRootTileCount);

    for(int x = 0; x < worldBlockWidth; x++)
    {
        for(int y = 0; y < worldBlockHeight; y++)
        {
            //int tileIndex = x * worldBlockHeight + y;
            AuroraTile* tile = new AuroraTile(maxBlockSize, Vector2(x * maxBlockSize, y * maxBlockSize));
            m_rootTiles.emplace_back(tile);
        }
    }

    printf("%lu tiles created\n", m_rootTiles.size());

    // Fill world

    // Air
    AuroraMaterial air;
    air.AddGazMaterial(MaterialQuantity(Material::Nitrogen, 78, 0));
    air.AddGazMaterial(MaterialQuantity(Material::Oxygen, 21, 0));
    air.AddGazMaterial(MaterialQuantity(Material::Water, 1, 0));
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

void TileComposition::AddLiquidVolume(LiquidMaterial material, Quantity quantity, Quantity dissolvedQuantity, Scalar temperature, Scalar pressure)
{
    TileLiquidVolume volume;
    volume.material = material;
    volume.quantity = quantity;
    volume.dissolvedQuantity = dissolvedQuantity;
    volume.temperature = temperature;
    volume.pressure = pressure;
     liquids.push_back(volume);
}

void TileComposition::AddSolidVolume(SolidMaterial material, Quantity quantity)
{
    TileSolidVolume volume;
    volume.material = material;
    volume.quantity = quantity;
    solids.push_back(volume);
}

}

