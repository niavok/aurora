#include "aurora_world.h"

namespace aurora {


scalar const AuroraWorld::MinTileSize = 0.0625;
scalar const AuroraWorld::TileChildEdgeCount = 2;
scalar const AuroraWorld::TileChildCount = AuroraWorld::TileChildEdgeCount * AuroraWorld::TileChildEdgeCount;

AuroraWorld::~AuroraWorld()
{
	for(AuroraTile* tile : m_rootTiles)
	{
		delete tile;
	}
}

void AuroraWorld::add(int value) {

    count += value;
}

void AuroraWorld::reset() {

    count = 0;
}

int AuroraWorld::get_total() const {

    return count;
}

void AuroraWorld::_bind_methods() {

	ClassDB::bind_method(D_METHOD("add", "value"), &AuroraWorld::add);
	ClassDB::bind_method(D_METHOD("reset"), &AuroraWorld::reset);
	ClassDB::bind_method(D_METHOD("get_total"), &AuroraWorld::get_total);
}

AuroraWorld::AuroraWorld() {
    count = 0;
	printf("plop1\n");
	Generate();
}

void AuroraWorld::Generate()
{
	int maxBlockSize = 256; // 256 meters

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

	Repack();
}

void AuroraWorld::PaintTile(Rect2 area, AuroraMaterial const& material)
{
	for(AuroraTile* tile : m_rootTiles)
	{
		tile->PaintTile(area, material);
	}
}

void AuroraWorld::Repack()
{
	for(AuroraTile* tile : m_rootTiles)
	{
		tile->Repack();
	}
}

Rect2 AuroraWorld::GetWorldArea() const
{
	return m_worldArea;
}

}

