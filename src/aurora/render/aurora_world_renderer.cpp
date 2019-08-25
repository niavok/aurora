#include "aurora_world_renderer.h"
#include "scene/2d/sprite.h"


#include "../world/aurora_world.h"
#include "../world/aurora_tile.h"
#include "../world/aurora_level.h"

#include <string>

namespace aurora {

static Scalar MnToGodot = 1.; // Mn in micrometer, Godot in Mm

AuroraWorldRenderer::AuroraWorldRenderer()
	: m_targetWorld(nullptr)
{
    m_control = memnew(Control);
    m_debugFont = m_control->get_font("mono_font", "Fonts");
}


AuroraWorldRenderer::~AuroraWorldRenderer()
{
    memdelete(m_control);

	if (m_testTexture1.is_valid())
		m_testTexture1->remove_change_receptor(this);

	if (m_testTexture2.is_valid())
		m_testTexture2->remove_change_receptor(this);
}

//if (!target_node_override && !target_node_path_override.is_empty())
//	target_node_override = Object::cast_to<Spatial>(get_node(target_node_path_override));

void AuroraWorldRenderer::set_world_node(const NodePath &p_node)
{

	printf("set_world_node p_node=%ls\n", String(p_node).c_str());


	m_targetWorldPath = p_node;
	m_targetWorld = nullptr;
}

NodePath AuroraWorldRenderer::get_world_node()
{
	return m_targetWorldPath;
}


void AuroraWorldRenderer::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_world_node", "node"), &AuroraWorldRenderer::set_world_node);
	ClassDB::bind_method(D_METHOD("get_world_node"), &AuroraWorldRenderer::get_world_node);

	ClassDB::bind_method(D_METHOD("set_texture1", "texture"), &AuroraWorldRenderer::set_texture1);
	ClassDB::bind_method(D_METHOD("get_texture1"), &AuroraWorldRenderer::get_texture1);
	ClassDB::bind_method(D_METHOD("set_texture2", "texture"), &AuroraWorldRenderer::set_texture2);
	ClassDB::bind_method(D_METHOD("get_texture2"), &AuroraWorldRenderer::get_texture2);



	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "world_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "AuroraWorld"), "set_world_node", "get_world_node");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture1", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture1", "get_texture1");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture2", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture2", "get_texture2");

}

void AuroraWorldRenderer::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_INTERNAL_PROCESS: {
            //printf("NOTIFICATION_INTERNAL_PROCESS\n");
            update();
            //_change_notify("frame");
            //emit_signal(SceneStringNames::get_singleton()->frame_changed);
		} break;
		case NOTIFICATION_TRANSFORM_CHANGED: {
		} break;
		case NOTIFICATION_ENTER_TREE: {
            set_process_internal(true);
		} break;
		case NOTIFICATION_DRAW: {
            //printf("NOTIFICATION_DRAW\n");
			if (m_testTexture1.is_null())
				return;
			if (m_testTexture2.is_null())
				return;

			if (has_node(m_targetWorldPath)) {
				m_targetWorld = Object::cast_to<AuroraWorld>(get_node(m_targetWorldPath));
			}
			else
			{
				printf("set_world_node fail to find node %ls\n", String(m_targetWorldPath).c_str());
				return;
			}



			if(m_targetWorld != nullptr)
			{
				RID ci = get_canvas_item();
				DrawWorld(ci);
			}


			/*Rect2 src_rect, dst_rect;
			bool filter_clip;
			_get_rects(src_rect, dst_rect, filter_clip);
			m_testTexture->draw_rect_region(ci, dst_rect, src_rect, Color(1, 1, 1), false, normal_map, filter_clip);*/
		} break;
	}
}


void AuroraWorldRenderer::set_texture1(const Ref<Texture> &p_texture) {

	if (p_texture == m_testTexture1)
		return;

	if (m_testTexture1.is_valid())
		m_testTexture1->remove_change_receptor(this);

	m_testTexture1 = p_texture;

	if (m_testTexture1.is_valid())
		m_testTexture1->add_change_receptor(this);

	update();
	emit_signal("texture_changed");
	item_rect_changed();
	_change_notify("texture");
}

Ref<Texture> AuroraWorldRenderer::get_texture1() const {

	return m_testTexture1;
}

void AuroraWorldRenderer::set_texture2(const Ref<Texture> &p_texture) {

	if (p_texture == m_testTexture2)
		return;

	if (m_testTexture2.is_valid())
		m_testTexture2->remove_change_receptor(this);

	m_testTexture2 = p_texture;

	if (m_testTexture2.is_valid())
		m_testTexture2->add_change_receptor(this);

	update();
	emit_signal("texture_changed");
	item_rect_changed();
	_change_notify("texture");
}

Ref<Texture> AuroraWorldRenderer::get_texture2() const {

	return m_testTexture2;
}

static int drawTileCount;

void AuroraWorldRenderer::DrawWorld(RID& ci)
{
    for(Level* level : m_targetWorld->GetLevels())
    {
        DrawLevel(ci, level);
    }

}

void AuroraWorldRenderer::DrawLevel(RID& ci, Level const* level)
{
	drawTileCount = 0;

    for(Tile const* tile : level->GetRootTiles())
	{
		DrawTile(ci, tile);
	}

	printf("tilecount=%d\n", drawTileCount);
}



void AuroraWorldRenderer::DrawTile(RID& ci, Tile const* tile)
{
	drawTileCount++;

	if(tile->IsComposite())
	{
        for(Tile const* child: tile->GetChildren())
		{
			DrawTile(ci, child);
		}
	}
	else
	{
        //printf("DrawTile at %ls size %f\n", String(tile->GetPosition()).c_str(), tile->GetSize());
        //m_testTexture->draw(ci,tile->GetPosition());

		Texture* texture;


		if(tile->GetContent()->HasSolid())
		{
			texture = *m_testTexture1;
		}
		else
		{
			texture = *m_testTexture2;
		}

        Vector2 pos = tile->GetPosition().ToVector2() * MnToGodot;
        real_t size = tile->GetSize() * MnToGodot;

        //texture->draw_rect(ci, Rect2(pos, Vector2(size, size)));



        if(pos.x < 1000 && pos.y < 1000)
        {
            Color color(0.9f,0.9f,0.9f);
            GasNode const& gas = tile->GetContent()->GetGazNode();

            Scalar pressure = gas.GetPressure();
            Scalar temperature = gas.GetTemperature();

            //gas.ComputeNPT(N, pressure, temperature);
            Scalar bottomPressure = pressure + gas.GetPressureGradient() * tile->GetSize();

            auto PressureToColor = [](Scalar pressure)
            {
                Scalar const minPressure = 90000;
                Scalar const maxPressure = 110000;

                return float((pressure-minPressure) / (maxPressure - minPressure));
            };

            float temperatureColor = float(temperature / 300.);
            float pressureColor = PressureToColor(pressure);
            float bottomPressureColor = PressureToColor(bottomPressure);



            Vector<Vector2> points;
            points.push_back(pos);
            points.push_back(pos + Vector2(size, 0));
            points.push_back(pos + Vector2(size, size));
            points.push_back(pos + Vector2(0, size));



            Color topColor(temperatureColor, 0, pressureColor);
            Color bottomColor(temperatureColor, 0, bottomPressureColor);

            Vector<Color> colors;


            colors.push_back(topColor);
            colors.push_back(topColor);
            colors.push_back(bottomColor);
            colors.push_back(bottomColor);
            //colors.push_back(Color(0, 0, 1, 0.5));


            draw_polygon(points, colors);

            m_debugFont->draw(ci, pos + Vector2(10, 20), rtos(pressure * 1e-5), color);
            m_debugFont->draw(ci, pos + Vector2(10, 40), rtos(temperature), color);

        }





		//virtual void draw_rect(RID p_canvas_item, const Rect2 &p_rect, bool p_tile = false, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, const Ref<Texture> &p_normal_map = Ref<Texture>()) const;


		//bool filter_clip;
		//_get_rects(src_rect, dst_rect, filter_clip);
		//m_testTexture->draw_rect_region(ci, dst_rect, src_rect, Color(1, 1, 1), false, normal_map, filter_clip);*/
	}
}

Rect2 AuroraWorldRenderer::_edit_get_rect() const
{
    if (m_testTexture1.is_valid() && m_testTexture2.is_valid() && m_targetWorld != nullptr && m_targetWorld->GetLevels().size() > 0)
	{
        Level* firstLevel = m_targetWorld->GetLevels()[0];
        return Rect2(Vector2(0, 0) , firstLevel->GetSize().ToVector2() * MnToGodot);
	}
	return Rect2(0, 0, 0, 0);
}

bool AuroraWorldRenderer::_edit_use_rect() const
{
	if (m_testTexture1.is_valid() && m_testTexture2.is_valid() && m_targetWorld != nullptr)
		return true;

	return false;
}

}
