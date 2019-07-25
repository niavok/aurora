#include "aurora_world.h"
#include "aurora_world_editor.h"
#include "aurora_level.h"

namespace aurora {

AuroraWorld::AuroraWorld() {
    //count = 0;
    printf("plop1\n");
    WorldEditor worldEditor(*this);
    worldEditor.GenerateHelloWord();
}

AuroraWorld::~AuroraWorld()
{
    for(Level* level : m_levels)
    {
        delete level;
    }
}

void AuroraWorld::_bind_methods() {
}


Level* AuroraWorld::CreateLevel(Mm minTileSize, int maxTileSubdivision, int rootTileHCount, int rootTileVCount)
{
    Level* level = new Level(minTileSize, maxTileSubdivision, rootTileHCount, rootTileVCount);
    m_levels.push_back(level);

    return level;
}


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

