#ifndef AURORA_TILE_H
#define AURORA_TILE_H

#include "core/math/vector2.h"
#include "core/math/rect2.h"
#include "tools/aurora_types.h"
#include <vector>

//class AuroraTile;

namespace aurora {

enum Material
{
	Void,
	Dirt,
	Limestone, // CaCO3
	Water, // H2O
	Oxygen, // O2
	Nitrogen, // N2
	CarbonDioxide, // CO2
	Methane,// CH4
	Wood,
	Charcoal,
	Coal,
	Coke,
};



struct MaterialQuantity
{
	MaterialQuantity(Material pMaterial, scalar pQuantity, scalar pQuality);

	Material material;
	scalar quantity;
	scalar quality;
};


typedef std::vector<MaterialQuantity> MaterialComposition;

bool operator ==(const MaterialComposition &a, const MaterialComposition &b);


/**
 * @brief The content of 1m2
 */
class AuroraMaterial
{

public:
	void SetSolidProperties(bool isPowder);

	void AddSolidMaterial(MaterialQuantity solidQuantity);
	void AddLiquidMaterial(MaterialQuantity liquidQuantity);
	void AddGazMaterial(MaterialQuantity gazQuantity);

	void SetTemperature(scalar temperature);

	std::vector<MaterialQuantity> const& GetSolidComposition() const;
	std::vector<MaterialQuantity> const& GetLiquidComposition() const;
	std::vector<MaterialQuantity> const& GetGazComposition() const;

private:
	bool m_isPowder;
	std::vector<MaterialQuantity> m_solidComposition;
	std::vector<MaterialQuantity> m_liquidComposition;
	std::vector<MaterialQuantity> m_gazComposition;

	scalar m_solidHeat;
	scalar m_liquidHeat;
	scalar m_gazHeat;
};


class AuroraTileContent {
public:
	AuroraTileContent(scalar volume);

	void SetSolidProperties(bool isPowder);

	/**
	 * @brief Fill the tile with a material
	 * @param material
	 */
	void SetMaterial(AuroraMaterial const& material);

	void SetContent(AuroraTileContent const& content);
	void ClearContent();

	AuroraTileContent Scaled(scalar ratio) const;
	void Scale(scalar ratio);

	void AddMaterial(AuroraMaterial const& material, scalar volume);
	void RemoveMaterial(scalar volume);

	void AddSolidQuantity(MaterialQuantity solidQuantity);
	void AddLiquidQuantity(MaterialQuantity liquidQuantity);
	void AddGazQuantity(MaterialQuantity gazQuantity);

	void AddGazComposition(MaterialComposition const& composition);
	void AddLiquidComposition(MaterialComposition const& composition);
	void AddSolidComposition(MaterialComposition const& composition);

	void SetTemperature(scalar temperature);

	bool HasSolid() const;

	MaterialComposition& GetSolidComposition();
	MaterialComposition& GetLiquidComposition();
	MaterialComposition& GetGazComposition();

private:
	scalar m_volume;
	scalar m_isPowder;

	MaterialComposition m_solidComposition;
	MaterialComposition m_liquidComposition;
	MaterialComposition m_gazComposition;

	scalar solidHeat;
	scalar liquidHeat;
	scalar gazHeat;
};


class AuroraTile  {
public:
	AuroraTile(scalar size, Vector2 position);
	~AuroraTile();

	enum InsideMode	{
		Yes,
		No,
		Partially,
	};

	InsideMode IsInside(Rect2 area);

	void PaintTile(Rect2 area, AuroraMaterial const& material);

	scalar GetVolume() const;

	/**
	 * @brief Make the tile composite if possible,
	 * @return return true if the tile is already composite or
	 * has been made composite
	 */
	bool SplitTile();

	/**
	 * @brief IsComposite
	 * @return Return true if the tile has children
	 */
	bool IsComposite() const;

	/**
	 * @brief Erase all the content with the given material
	 * @param material used as material so it will be scaled
	 */
	void PaintContent(AuroraMaterial const& material);

	/**
	 * @brief Merge composite tiles with same composition
	 */
	void Repack();

	/**
	 * @brief Erase all the content and replace with the given content
	 * @param content used as quantity so it won't be scaled
	 */
	void SetContent(AuroraTileContent const& content);

	Vector2 GetPosition() const { return m_position; }

	scalar GetSize() const { return m_size; }

	std::vector<AuroraTile*> const& GetChildren() const { return m_children; }

	std::vector<AuroraTile*>& GetChildren() { return m_children; }

	AuroraTileContent* GetContent() { return m_content; }
	AuroraTileContent const* GetContent() const { return m_content; }

private:




	scalar m_size;
	Vector2 m_position; // Optionnal ?
	std::vector<AuroraTile*> m_children;
	AuroraTileContent* m_content;

	// Cache
	Rect2 m_worldArea;
};

}

#endif
