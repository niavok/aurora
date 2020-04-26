extends Control

export(NodePath) var aurora_world_renderer_path
export(NodePath) var aurora_game_manager_path

var world_render_stats_label : Label
var world_update_stats_label : Label
var world_renderer : AuroraWorldRenderer
var game_manager : AuroraGameManager

var smooth_last_render_duration_us : float
# TODO max
var has_game : bool

# Called when the node enters the scene tree for the first time.
func _ready():
	self.world_render_stats_label = get_node("DebugPanel/WorldRenderStatsLabel")
	self.world_update_stats_label = get_node("DebugPanel/WorldUpdateStatsLabel")
	self.world_renderer = get_node(self.aurora_world_renderer_path)
	self.game_manager = get_node(self.aurora_game_manager_path)
	has_game = false
	self.update_state()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	var render_duration_us = world_renderer.get_last_render_duration()
	if(game_manager.has_game()):
		var update_duration_us = game_manager.get_game().get_world().get_last_update_duration()
		world_update_stats_label.text = "World update: %5.2f ms" % (update_duration_us * 0.001)
	else:
		world_update_stats_label.text = "World update: no world"

	self.smooth_last_render_duration_us = lerp(self.smooth_last_render_duration_us, render_duration_us, 0.9)

	world_render_stats_label.text = "World render: %5.2f ms" % (self.smooth_last_render_duration_us * 0.001)
	

	var new_has_game = game_manager.has_game()
	if new_has_game != self.has_game:
		self.has_game = new_has_game
		self.update_state()

func update_state():
	var save_game_button = get_node("MainMenu/VBoxContainer/SaveButton")
	var load_game_button = get_node("MainMenu/VBoxContainer/LoadButton")
	save_game_button.disabled = !self.has_game
	load_game_button.disabled = !self.has_game
	
	var editor_panel = get_node("EditorPanel")
	editor_panel.set_visible(self.has_game)

func _on_SaveButton_pressed():
	self.game_manager.get_game().save()

func _on_LoadButton_pressed():
	self.game_manager.load(0)

func _on_QuitButton_pressed():
	get_tree().quit()





func _on_NewButton_pressed():
	self.game_manager.create_game()
