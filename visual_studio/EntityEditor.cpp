#include "EntityEditor.h"
#include "extern.h"
#include "Parsers.h"
#include "shaders_default.h"


void DebugSystem::init(GraphicsSystem* gs) {
	graphics_system_ = gs;
}

void DebugSystem::lateInit() {
	//init booleans
	draw_grid_ = false;
	draw_icons_ = false;
	draw_frustra_ = false;
	draw_colliders_ = false;

	//compile debug shaders from strings in header file
	grid_shader_ = new Shader();
	grid_shader_->compileFromStrings(g_shader_line_vertex, g_shader_line_fragment);
	icon_shader_ = new Shader();
	icon_shader_->compileFromStrings(g_shader_icon_vertex, g_shader_icon_fragment);

	//create geometries
	createGrid_();
	createIcon_();
	createCube_();
	createRay_();

	//create texture for light icon
	icon_light_texture_ = Parsers::parseTexture("data/assets/icon_light.tga");
	icon_camera_texture_ = Parsers::parseTexture("data/assets/icon_camera.tga");

	//picking collider
	ent_picking_ray_ = ECS.createEntity("picking_ray");
	Collider& picking_ray = ECS.createComponentForEntity<Collider>(ent_picking_ray_);
	picking_ray.collider_type = ColliderTypeRay;
	picking_ray.direction = lm::vec3(0, 0, -1);
	picking_ray.max_distance = 0.001f;

	//bones
	joint_shader_ = new Shader("data/shaders/joints.vert", "data/shaders/joints.frag");
	createJointGeometry_();

	setActive(true);
}

//draws debug information or not
void DebugSystem::setActive(bool a) {
	active_ = a;
	draw_grid_ = a;
	draw_icons_ = a;
	draw_frustra_ = a;
	draw_colliders_ = a;
	draw_joints_ = a;
}

//called once per frame
void DebugSystem::update(float dt) {

	if (!active_) return;

	//line drawing first, use same shader
	if (draw_grid_ || draw_frustra_ || draw_colliders_) {

		//use line shader to draw all lines and boxes
		glUseProgram(grid_shader_->program);

		if (draw_grid_) {
			drawGrid_();
		}

		if (draw_frustra_) {
			drawFrusta_();
		}

		if (draw_colliders_) {
			drawColliders_();
		}
	}

	//icon drawing
	if (draw_icons_) {
		drawIcons_();
	}
	//joint drawing
	if (draw_joints_) {
		drawJoints_();
	}


	glBindVertexArray(0);

	//imGUI
	updateimGUI_(dt);
}