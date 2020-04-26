extends PanelContainer

export(NodePath) var gui_root_path

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var gui_root : Control
var level_list : OptionButton
var levels_version = -1

# Called when the node enters the scene tree for the first time.
func _ready():
	self.gui_root = get_node(gui_root_path)
	self.level_list = get_node("./VBoxContainer/LevelSelectionButton")

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if not gui_root.game_manager.has_game():
		return
	
	var world = gui_root.game_manager.get_game().get_world()
	var old_levels_version = self.levels_version
	self.levels_version = world.get_levels_version()
	
	if self.levels_version != old_levels_version:
		update_level_list()

func _on_NewLevelButton_pressed():
	if not gui_root.game_manager.has_game():
		return
	
	var world = gui_root.game_manager.get_game().get_world()
	var world_editor = gui_root.game_manager.get_game().get_world_editor()
	var level = world_editor.create_empty_level()
	
	update_level_list();
	self.level_list.selected = world.get_level_count() -1
	self.levels_version = world.get_levels_version()

func update_level_list():
	
	var world = gui_root.game_manager.get_game().get_world()
	var levels = world.get_levels()
	
	if self.level_list.get_item_count() != len(levels):
		self.level_list.selected =-1
	
	self.level_list.clear()
	for level in levels:
		self.level_list.add_item(level.get_name())
	
	
	
