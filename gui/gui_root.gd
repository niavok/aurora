extends Control

export(String) var aurora_world_name
export(String) var aurora_world_renderer_name

var world_render_stats_label : Label
var world_update_stats_label : Label
var aurora_world : AuroraWorld
var aurora_world_renderer : AuroraWorldRenderer

# Called when the node enters the scene tree for the first time.
func _ready():
	self.world_render_stats_label = get_node("DebugPanel/WorldRenderStatsLabel")
	self.world_update_stats_label = get_node("DebugPanel/WorldUpdateStatsLabel")
	self.aurora_world = get_node(self.aurora_world_name)
	self.aurora_world_renderer = get_node(self.aurora_world_renderer_name)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	var renderDurationUs = aurora_world_renderer.get_last_render_duration()
	var updateDurationUs = aurora_world.get_last_update_duration()

	world_render_stats_label.text = "World render: "+ str(renderDurationUs * 0.001) + " ms"
	world_update_stats_label.text = "World update: "+ str(updateDurationUs * 0.001) + " ms"
