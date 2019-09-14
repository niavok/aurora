extends KinematicBody2D

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var world : Node2D

# Called when the node enters the scene tree for the first time.
func _ready():
	world = get_node("/root/world")

func _physics_process(delta):
	
	var velocityDirection = Vector2()
	var speed = 10000
	
	#print("FPS=" + String(Engine.get_frames_per_second()))
	if not world.use_free_camera:
		if Input.is_action_pressed("move_right"):
			velocityDirection.x+=1
		if Input.is_action_pressed("move_left"):
			velocityDirection.x+=-1
		if Input.is_action_pressed("move_up"):
			velocityDirection.y+=-1
		if Input.is_action_pressed("move_down"):
			velocityDirection.y+=1
		
		var deltaPos = speed * velocityDirection * delta
		print("player camera " + String(deltaPos) + " fps=" + String(Engine.get_frames_per_second()))
		move_and_slide(deltaPos)
