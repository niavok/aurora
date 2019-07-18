extends Camera2D

var world : Node2D

# Called when the node enters the scene tree for the first time.
func _ready():
	world = get_node("/root/world")
	self.drag_margin_h_enabled = false
	self.drag_margin_v_enabled = false


func _unhandled_input(event):
	if event is InputEventMouseButton:
		if event.is_pressed() and event.button_index == BUTTON_WHEEL_DOWN:
			zoom_out(1)
		if event.is_pressed() and event.button_index == BUTTON_WHEEL_UP:
			zoom_in(1)

func _physics_process(delta):
	
	var velocityDirection = Vector2()
	var speed = 1000 * self.zoom
	
	if world.use_free_camera:
		if Input.is_action_pressed("move_right"):
			velocityDirection.x+=1
		if Input.is_action_pressed("move_left"):
			velocityDirection.x+=-1
		if Input.is_action_pressed("move_up"):
			velocityDirection.y+=-1
		if Input.is_action_pressed("move_down"):
			velocityDirection.y+=1
		
		var deltaPos = speed * velocityDirection * delta
		#print("free camera " + String(deltaPos) + " fps=" + String(Engine.get_frames_per_second()))
		
		self.position += deltaPos
		
		var zoom_ratio = 1
		
		if Input.is_action_pressed("zoom_in"):
			zoom_in(delta)
		if Input.is_action_pressed("zoom_out"):
			zoom_out(delta)

func zoom_in(amount):
	# TODO: zoom to the cursor
	# TODO: smooth zoom
	var zoom_ratio = 1.0/(1.0 + amount)
	self.zoom *= zoom_ratio
	if self.zoom.x < 0.5:
		self.zoom = self.zoom * (0.5 / self.zoom.x) 
	print("zoom "+String(zoom)+" ratio "+String(zoom))

func zoom_out(amount):
	var zoom_ratio = 1.0 + amount
	self.zoom *= zoom_ratio
	if self.zoom.x > 1000:
			self.zoom = self.zoom * (1000 / self.zoom.x)
	print("zoom "+String(zoom)+" ratio "+String(zoom))
