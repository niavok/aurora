#ifndef AURORA_WORLD_H
#define AURORA_WORLD_H

#include "scene/main/node.h"

#include "aurora_tile.h"
#include <vector>

namespace aurora {

class Level;

class AuroraWorld : public Node {
    GDCLASS(AuroraWorld, Node)

    virtual ~AuroraWorld();

    int count;

protected:
    static void _bind_methods();

public:
    //static scalar const MinTileSize;
    //static scalar const TileChildEdgeCount;
    //static scalar const TileChildCount;

    // TODO test
    //void add(int value);
    //void reset();
    //int get_total() const;

    AuroraWorld();

    Level* CreateLevel(Unit minTileSize, int maxTileSubdivision, int rootTileHCount, int rootTileVCount);

    //void PaintTile(Rect2 area, AuroraMaterial const& material);

    //void Repack();

    //std::vector<AuroraTile*>& GetRootTiles() { return m_rootTiles; }
    //std::vector<AuroraTile*> const& GetRootTiles() const { return m_rootTiles; }

    //Rect2 GetWorldArea() const;
    std::vector<Level*>& GetLevels() { return m_levels; }
    std::vector<Level*> const& GetLevels() const { return m_levels; }

private:
    std::vector<Level*> m_levels;
    //std::vector<Tile*> m_rootTiles;
    //unitRect m_worldArea;
};

}

#endif
