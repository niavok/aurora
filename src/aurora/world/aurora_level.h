#ifndef AURORA_LEVEL_H
#define AURORA_LEVEL_H


#include "aurora_tile.h"
#include <vector>

namespace aurora {

class Level {

public:
    //static scalar const MinTileSize;
    //static scalar const TileChildEdgeCount;
    //static scalar const TileChildCount;

    // TODO test
    //void add(int value);
    //void reset();
    //int get_total() const;

    Level();

    //void PaintTile(Rect2 area, AuroraMaterial const& material);

    //void Repack();

    std::vector<Tile*>& GetRootTiles() { return m_rootTiles; }
    std::vector<Tile*> const& GetRootTiles() const { return m_rootTiles; }

    Unit2 GetSize() const;

    //Rect2 GetArea() const;

private:
    std::vector<Tile*> m_rootTiles;
    Unit2 m_size;
};

}

#endif
