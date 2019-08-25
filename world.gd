extends Node2D

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var free_camera : Camera2D
var player_camera : Camera2D
var use_free_camera = true
var world = AuroraWorld

# Called when the node enters the scene tree for the first time.
func _ready():
	free_camera = get_node("FreeCamera")
	player_camera = get_node("test_player/Camera")
	world = get_node("AuroraWorld")
	update_current_camera()
	pass # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if Input.is_action_just_pressed("toogle_free_camera"):
		use_free_camera = !use_free_camera
		update_current_camera()
		
	if Input.is_action_just_pressed("game_step"):
		world.step()

	if Input.is_action_just_pressed("game_toogle_pause"):
		world.set_pause(not world.is_paused())

func update_current_camera():
	if use_free_camera:
		free_camera.make_current()
	else:
		player_camera.make_current()
