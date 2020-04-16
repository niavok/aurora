extends Control

export(NodePath) var aurora_world_path
export(NodePath) var aurora_world_renderer_path
export(NodePath) var aurora_world_editor_path

var world_render_stats_label : Label
var world_update_stats_label : Label
var aurora_world : AuroraWorld
var aurora_world_renderer : AuroraWorldRenderer
var aurora_world_editor : AuroraWorldEditor

var smooth_last_render_duration_us : float
# TODO max


# Called when the node enters the scene tree for the first time.
func _ready():
	self.world_render_stats_label = get_node("DebugPanel/WorldRenderStatsLabel")
	self.world_update_stats_label = get_node("DebugPanel/WorldUpdateStatsLabel")
	self.aurora_world = get_node(self.aurora_world_path)
	self.aurora_world_renderer = get_node(self.aurora_world_renderer_path)
	self.aurora_world_editor = get_node(self.aurora_world_editor_path)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	var render_duration_us = aurora_world_renderer.get_last_render_duration()
	var update_duration_us = aurora_world.get_last_update_duration()

	self.smooth_last_render_duration_us = lerp(self.smooth_last_render_duration_us, render_duration_us, 0.9)

	world_render_stats_label.text = "World render: %5.2f ms" % (self.smooth_last_render_duration_us * 0.001)
	world_update_stats_label.text = "World update: %5.2f ms" % (update_duration_us * 0.001)
