#include "aurora_tile.h"
#include "aurora_world.h"

namespace aurora {

AuroraTile::AuroraTile(scalar size, Vector2 position)
	: m_size(size)
	, m_position(position)
	, m_worldArea(position, Vector2(size, size)) {

	m_content = new AuroraTileContent(GetVolume()); // TODO optimized in case of tile that will be splitted
	//printf("AuroraTile new AuroraTileContent %p\n", m_content);
}


AuroraTile::~AuroraTile()
{
	if(m_content)
	{
		//printf("AuroraTile delete AuroraTileContent %p\n", m_content);
		delete m_content;
	}
}


void AuroraTile::PaintTile(Rect2 area, AuroraMaterial const& material)
{
	//printf("PaintTile area=%ls tile_area=%ls\n", String(area).c_str(), String(m_worldArea).c_str());

	switch(IsInside(area))
	{
	case InsideMode::Yes:
	{
		//printf("inside: Yes\n");
		// All the tile is in the target area, put the material everywhere
		PaintContent(material);
	}
	break;
	case InsideMode::Partially:
	{
		//printf("inside: Partially\n");
		if(SplitTile())
		{
			//printf("split ok\n");
			for(AuroraTile* child: m_children)
			{
				//printf("split child\n");
				child->PaintTile(area, material);
			}
			//printf("split done\n");
		}
		else
		{
			//printf("split ko\n");
			// Cannot split more, already at min size, merge
			Rect2 intersection = m_worldArea.clip(area);

			scalar volume = GetVolume();
			scalar mergeRatio = intersection.get_area() / volume;

			scalar newVolume = volume * mergeRatio;

			m_content->RemoveMaterial(newVolume);
			m_content->AddMaterial(material, newVolume);
		}
	}
	break;
	case InsideMode::No:
		//printf("inside: No\n");
	default:
	break;
	}
}

void AuroraTile::Repack()
{
	if(!IsComposite())
	{
		// Nothing to repack
		return;
	}

	bool hasCompositeChildAfterRepack = false;

	// Try to repack child
	for(AuroraTile* child: m_children)
	{
		child->Repack();
		if(child->IsComposite())
		{
			hasCompositeChildAfterRepack = true;
		}
	}

	if(!hasCompositeChildAfterRepack)
	{
		MaterialComposition& solidComposition = m_children[0]->GetContent()->GetSolidComposition();

		bool isSameSolidComposition = true;

		for(AuroraTile* child: m_children)
		{
			if(child->GetContent()->GetSolidComposition() != solidComposition)
			{
				isSameSolidComposition = false;
				break;
			}
		}

		if(isSameSolidComposition)
		{
			AuroraTileContent mergedContent(GetVolume());

			for(AuroraTile* child: m_children)
			{
				mergedContent.AddGazComposition(child->GetContent()->GetGazComposition());
				mergedContent.AddLiquidComposition(child->GetContent()->GetLiquidComposition());
				mergedContent.AddSolidComposition(child->GetContent()->GetSolidComposition());
				// TODO merge heat
			}
			SetContent(mergedContent);
		}

	}



}

scalar AuroraTile::GetVolume() const
{
	return sqr(m_size);
}

bool AuroraTile::IsComposite() const
{
	return m_content == nullptr;
}

void AuroraTile::PaintContent(AuroraMaterial const& material)
{
	if(IsComposite())
	{
		for(AuroraTile* child: m_children)
		{
			delete child;
		}
		m_children.clear();
		m_content = new AuroraTileContent(GetVolume());
	}

	m_content->SetMaterial(material);
}

void AuroraTile::SetContent(AuroraTileContent const& content)
{
	if(IsComposite())
	{
		for(AuroraTile* child: m_children)
		{
			delete child;
		}
		m_children.clear();
		m_content = new AuroraTileContent(GetVolume());
	}

	m_content->SetContent(content);
}

bool AuroraTile::SplitTile()
{
	if(IsComposite())
	{
		//printf("Split tile : alreaydy composite\n");
		// Already composite
		return true;
	}
	else if(m_size <= AuroraWorld::MinTileSize)
	{
		//printf("Split tile : too small %f\n", m_size);
		// Too small, cannot split
		return false;
	}
	else
	{

		scalar childrenSize = m_size / AuroraWorld::TileChildEdgeCount;

		//printf("do split childrenSize=%f\n", childrenSize);

		AuroraTileContent childContent = m_content->Scaled(1 / AuroraWorld::TileChildCount);
		for(int y = 0; y < AuroraWorld::TileChildEdgeCount; y++)
		{
			for(int x = 0; x < AuroraWorld::TileChildEdgeCount; x++)
			{
				Vector2 position = m_position + Vector2(x * childrenSize, y * childrenSize);

				AuroraTile* newTile = new AuroraTile(childrenSize, position);
				m_children.emplace_back(newTile);
				newTile->SetContent(childContent);

				//printf("newTile pos=%ls\n", String(newTile->GetPosition()).c_str());

			}
		}

		delete m_content;
		m_content = nullptr;

		return true;
	}
}

AuroraTile::InsideMode AuroraTile::IsInside(Rect2 area)
{
	Rect2 intersection = area.clip(m_worldArea);

	if(intersection.has_no_area())
	{
		return No;
	}
	else if(intersection.size == m_worldArea.size)
	{
		return Yes;
	}
	else
	{
		return Partially;
	}
}



////////////////////////
/// MaterialQuantity
////////////////////////

MaterialQuantity::MaterialQuantity(Material pMaterial, scalar pQuantity, scalar pQuality)
	: material(pMaterial)
	, quantity(pQuantity)
	, quality(pQuality)
{
}

////////////////////////
/// AuroraMaterial
////////////////////////

void AuroraMaterial::SetSolidProperties(bool isPowder)
{
	m_isPowder = isPowder;
}


static void AddInComposition(MaterialQuantity& materialQuantity, std::vector<MaterialQuantity>& composition)
{
	for(MaterialQuantity& quantity : composition)
	{
		if(quantity.material == materialQuantity.material)
		{
			scalar newQuantity = materialQuantity.quantity + quantity.quantity;
			scalar newQuality = (materialQuantity.quality * materialQuantity.quantity + quantity.quality * quantity.quantity) / newQuantity;

			quantity.quantity = newQuantity;
			quantity.quality = newQuality;
			return;
		}
	}

	// Not found
	composition.emplace_back(materialQuantity);
}

void AuroraMaterial::AddSolidMaterial(MaterialQuantity solidQuantity)
{
	AddInComposition(solidQuantity, m_solidComposition);
}
void AuroraMaterial::AddLiquidMaterial(MaterialQuantity liquidQuantity)
{
	AddInComposition(liquidQuantity, m_liquidComposition);
}
void AuroraMaterial::AddGazMaterial(MaterialQuantity gazQuantity)
{
	AddInComposition(gazQuantity, m_gazComposition);
}

void AuroraMaterial::SetTemperature(scalar temperature)
{
	// TODO
}

std::vector<MaterialQuantity> const& AuroraMaterial::GetSolidComposition() const
{
	return m_solidComposition;
}

std::vector<MaterialQuantity> const& AuroraMaterial::GetLiquidComposition() const
{
	return m_liquidComposition;
}

std::vector<MaterialQuantity> const& AuroraMaterial::GetGazComposition() const
{
	return m_gazComposition;
}

////////////////////////
/// AuroraTileContent
////////////////////////


AuroraTileContent::AuroraTileContent(scalar volume)
	: m_volume(volume)
{

}

void AuroraTileContent::SetSolidProperties(bool isPowder)
{
	m_isPowder = isPowder;
}

void AuroraTileContent::SetMaterial(AuroraMaterial const& material)
{
	ClearContent();
	AddMaterial(material, m_volume);
}

void AuroraTileContent::SetContent(AuroraTileContent const& content)
{
	*this = content;
}

void AuroraTileContent::ClearContent()
{
	m_solidComposition.clear();
	m_liquidComposition.clear();
	m_gazComposition.clear();
}

static void ScaleComposition(std::vector<MaterialQuantity>& composition, scalar scaleRatio)
{
	for(MaterialQuantity& quantity : composition)
	{
		quantity.quantity *= scaleRatio;
	}
}

AuroraTileContent AuroraTileContent::Scaled(scalar ratio) const
{
	AuroraTileContent newContent = *this;
	newContent.Scale(ratio);
	return newContent;
}

void AuroraTileContent::AddMaterial(AuroraMaterial const& material, scalar volume)
{
	for(MaterialQuantity quantity : material.GetSolidComposition())
	{
		quantity.quantity *= volume;
		AddInComposition(quantity, m_solidComposition);
	}

	for(MaterialQuantity quantity : material.GetLiquidComposition())
	{
		quantity.quantity *= volume;
		AddInComposition(quantity, m_liquidComposition);
	}

	for(MaterialQuantity quantity : material.GetGazComposition())
	{
		quantity.quantity *= volume;
		AddInComposition(quantity, m_gazComposition);
	}
}

void AuroraTileContent::Scale(scalar ratio)
{
	m_volume *= ratio;
	ScaleComposition(m_solidComposition, ratio);
	ScaleComposition(m_liquidComposition, ratio);
	ScaleComposition(m_gazComposition, ratio);
}

void AuroraTileContent::RemoveMaterial(scalar volume)
{
	scalar remainingRatio = 1 - volume/m_volume;
	Scale(remainingRatio);
}

void AuroraTileContent::AddSolidQuantity(MaterialQuantity solidQuantity)
{
	AddInComposition(solidQuantity, m_solidComposition);
}

void AuroraTileContent::AddLiquidQuantity(MaterialQuantity liquidQuantity)
{
	AddInComposition(liquidQuantity, m_liquidComposition);
}
void AuroraTileContent::AddGazQuantity(MaterialQuantity gazQuantity)
{
	AddInComposition(gazQuantity, m_gazComposition);
}

void AuroraTileContent::SetTemperature(scalar temperature)
{
	//TODOT
}

bool AuroraTileContent::HasSolid() const
{
	// TODO Cache
	return !m_solidComposition.empty();
}

MaterialComposition& AuroraTileContent::GetSolidComposition()
{
	return m_solidComposition;
}
MaterialComposition& AuroraTileContent::GetLiquidComposition()
{
	return m_liquidComposition;
}

MaterialComposition& AuroraTileContent::GetGazComposition()
{
	return m_gazComposition;
}



}
