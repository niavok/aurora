#include "aurora_world.h"
#include "aurora_world_editor.h"
#include "aurora_level.h"

namespace aurora {

AuroraWorld::AuroraWorld() {
    //count = 0;
    printf("plop1\n");
    WorldEditor worldEditor(*this);
    worldEditor.GenerateHelloWord();

    InitPhysics();
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


void AuroraWorld::_notification(int p_what) {

     switch (p_what) {
        case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
            Variant delta = get_physics_process_delta_time();
            Update(delta);
        } break;
        case NOTIFICATION_ENTER_TREE: {
            set_physics_process_internal(true);
        } break;
    }
}


void AuroraWorld::Update(Scalar delta)
{
    m_physicEngine.Step(delta);
}



Level* AuroraWorld::CreateLevel(Mm minTileSize, int maxTileSubdivision, int rootTileHCount, int rootTileVCount)
{
    Level* level = new Level(minTileSize, maxTileSubdivision, rootTileHCount, rootTileVCount);
    m_levels.push_back(level);

    return level;
}

void AuroraWorld::InitPhysics()
{
    m_physicEngine.Flush();

    for(Level* level : m_levels)
    {
        for(Tile* tile : level->GetRootTiles())
        {
            m_physicEngine.AddFluidNode(&tile->GetContent()->GetGazNode());


            {
                MmRect tileRight;

                tileRight.position.y = tile->GetPosition().y;
                if(tile->GetPosition().x == 0)
                {
                    // Make world circular
                    tileRight.position.x = level->GetSize().x - 1;
                }
                else
                {
                    tileRight.position.x = tile->GetPosition().x + tile->GetSize();
                }

                tileRight.size.x = 1;
                tileRight.size.y = tile->GetSize();


                std::vector<Tile*> tilesToConnect;
                level->FindTileAt(tilesToConnect, tileRight);
                for(Tile* tileToConnect : tilesToConnect)
                {
                    Meter relativeAltitudeB;
                    Meter relativeAltitudeA;
                    if(tileToConnect->GetSize() < tile->GetSize())
                    {
                        relativeAltitudeB = tileToConnect->GetSize() / 2;
                        relativeAltitudeA = tileToConnect->GetPosition().y - tile->GetPosition().y + relativeAltitudeB;
                    }
                    else
                    {
                        relativeAltitudeA = tile->GetSize() / 2;
                        relativeAltitudeB = tile->GetPosition().y - tileToConnect->GetPosition().y + relativeAltitudeA;
                    }

                    Meter section = MIN(tileToConnect->GetSize(), tile->GetSize());
                    ConnectTiles(tile, tileToConnect, Transition::DIRECTION_RIGHT, relativeAltitudeA, relativeAltitudeB, section);
                }
            }



            {
                MmRect tileBottom;
                tileBottom.position.x = tile->GetPosition().x;
                tileBottom.position.y = tile->GetPosition().y + tile->GetSize();
                tileBottom.size.x = tile->GetSize();
                tileBottom.size.y = 1;

                std::vector<Tile*> tilesToConnect;
                level->FindTileAt(tilesToConnect, tileBottom);
                for(Tile* tileToConnect : tilesToConnect)
                {
                    Meter section = MIN(tileToConnect->GetSize(), tile->GetSize());
                    ConnectTiles(tile, tileToConnect, Transition::DIRECTION_DOWN, tile->GetSize(), 0, section);
                }
            }
        }
    }

    m_physicEngine.CheckDuplicates();
}

void AuroraWorld::ConnectTiles(Tile* tileA, Tile* tileB, Transition::Direction direction, Meter relativeAltitudeA, Meter relativeAltitudeB, Meter section)
{
    GasGasTransition::Config config(tileA->GetContent()->GetGazNode(), tileB->GetContent()->GetGazNode());
    config.relativeAltitudeA = relativeAltitudeA;
    config.relativeAltitudeB = relativeAltitudeB;
    config.direction = direction;
    config.section = section;

    Transition* transition = new GasGasTransition(config);
    m_physicEngine.AddTransition(transition);
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

