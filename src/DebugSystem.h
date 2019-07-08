#pragma once
#include "includes.h"
#include "Shader.h"
#include <vector>
#include "GraphicsSystem.h"
#include "EntityEditor.h"




class DebugSystem {
public:
	
	~DebugSystem();
	void init(GraphicsSystem* gs, EntityEditor* ee);
	void lateInit();
	void update(float dt);

	void setActive(bool a);

	//public imGUI functions


private:
	//graphics system pointer
	GraphicsSystem* graphics_system_;
	EntityEditor* entity_editor_;
	
	//bools to draw or not
    bool active_;
    bool draw_grid_;
    bool draw_icons_;
    bool draw_frustra_;
    bool draw_colliders_;
    bool draw_joints_;

	//cube for frustra and boxes
	void createCube_();
	GLuint cube_vao_;

	//colliders
	void createRay_();
	GLuint collider_ray_vao_;
	GLuint collider_box_vao_;

	//icons
	void createIcon_();
	GLuint icon_vao_;
	GLuint icon_light_texture_;
	GLuint icon_camera_texture_;

	//grid
	void createGrid_();
	GLuint grid_vao_;
	GLuint grid_num_indices;
	float grid_colors[12] = {
		0.7f, 0.7f, 0.7f, //grey
		1.0f, 0.5f, 0.5f, //red
		0.5f, 1.0f, 0.5f, //green
		0.5f, 0.5f, 1.0f }; //blue

							//shaders
	Shader* grid_shader_;
	Shader* icon_shader_;

    //drawing methods
    void drawGrid_();
    void drawIcons_();
    void drawFrusta_();
    void drawColliders_();
    void drawJoints_();
    

	//picking
	bool can_fire_picking_ray_ = true;
	int ent_picking_ray_;
    
    //bones
    std::vector<GLuint> joints_vaos_;
    std::vector<GLuint> joints_chain_counts_;
    void createJointGeometry_();
    void getJointWorldMatrices_(Joint* current,
                                lm::mat4 current_model,
                                std::vector<float>& all_matrices,
                                int& joint_count);
    Shader* joint_shader_;
	
};

