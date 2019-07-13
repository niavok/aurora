#ifndef AURORA_WORLD_H
#define AURORA_WORLD_H

#include "scene/main/node.h"
#include "../tools/aurora_types.h"

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
    AuroraWorld();

    Level* CreateLevel(Mm minTileSize, int maxTileSubdivision, int rootTileHCount, int rootTileVCount);

    std::vector<Level*>& GetLevels() { return m_levels; }
    std::vector<Level*> const& GetLevels() const { return m_levels; }

private:
    std::vector<Level*> m_levels;
};

}

#endif
