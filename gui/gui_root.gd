extends Control

export(String) var aurora_world_name
export(String) var aurora_world_renderer_name

var world_render_stats_label : Label
var physic_step_stats_label : Label
var aurora_world : AuroraWorld
var aurora_world_renderer : AuroraWorldRenderer

# Called when the node enters the scene tree for the first time.
func _ready():
	self.world_render_stats_label = get_node("DebugPannel/WorldRenderStatsLabel")
	self.physic_step_stats_label = get_node("DebugPannel/PhysicStepStatsLabel")
	self.aurora_world = get_node(self.aurora_world_name)
	self.aurora_world_renderer = get_node(self.aurora_world_renderer_name)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
