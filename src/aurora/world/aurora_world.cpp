#include "aurora_world.h"
#include "aurora_world_editor.h"

namespace aurora {


//scalar const AuroraWorld::MinTileSize = 0.0625;
//scalar const AuroraWorld::TileChildEdgeCount = 2;
//scalar const AuroraWorld::TileChildCount = AuroraWorld::TileChildEdgeCount * AuroraWorld::TileChildEdgeCount;

AuroraWorld::AuroraWorld() {
    //count = 0;
    printf("plop1\n");
    WorldEditor worldEditor(*this);
    worldEditor.GenerateTestWord();
}

AuroraWorld::~AuroraWorld()
{
//	for(AuroraTile* tile : m_rootTiles)
//	{
//		delete tile;
//	}
}

//void AuroraWorld::add(int value) {

//    count += value;
//}

//void AuroraWorld::reset() {

//    count = 0;
//}

//int AuroraWorld::get_total() const {

//    return count;
//}

void AuroraWorld::_bind_methods() {

    //ClassDB::bind_method(D_METHOD("add", "value"), &AuroraWorld::add);
    //ClassDB::bind_method(D_METHOD("reset"), &AuroraWorld::reset);
    //ClassDB::bind_method(D_METHOD("get_total"), &AuroraWorld::get_total);
}


Level* AuroraWorld::CreateLevel(Unit minTileSize, int maxTileSubdivision, int rootTileHCount, int rootTileVCount)
{
    // TODO
    return nullptr;
}

//void AuroraWorld::Generate()
//{

//}

//void AuroraWorld::PaintTile(Rect2 area, AuroraMaterial const& material)
//{
//	for(AuroraTile* tile : m_rootTiles)
//	{
//		tile->PaintTile(area, material);
//	}
//}

//void AuroraWorld::Repack()
//{
//	for(AuroraTile* tile : m_rootTiles)
//	{
//		tile->Repack();
//	}
//}

//Rect2 AuroraWorld::GetWorldArea() const
//{
//	return m_worldArea;
//}

}

