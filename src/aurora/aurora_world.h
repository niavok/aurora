#ifndef AURORA_WORLD_H
#define AURORA_WORLD_H

#include "scene/main/node.h"

#include "aurora_tile.h"
#include <vector>

namespace aurora {

class AuroraWorld : public Node {
	GDCLASS(AuroraWorld, Node);

	virtual ~AuroraWorld();

    int count;

protected:
    static void _bind_methods();

public:
	static scalar const MinTileSize;
	static scalar const TileChildEdgeCount;
	static scalar const TileChildCount;


    void add(int value);
    void reset();
    int get_total() const;

	AuroraWorld();

	void Generate();

	void PaintTile(Rect2 area, AuroraMaterial const& material);

	void Repack();

	std::vector<AuroraTile*>& GetRootTiles() { return m_rootTiles; }
	std::vector<AuroraTile*> const& GetRootTiles() const { return m_rootTiles; }

	Rect2 GetWorldArea() const;

private:
	std::vector<AuroraTile*> m_rootTiles;
	Rect2 m_worldArea;
};

}

#endif
