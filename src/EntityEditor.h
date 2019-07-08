#pragma once
#include "includes.h"
#include "Shader.h"
#include <vector>
#include "GraphicsSystem.h"

struct TransformNode {
	std::vector<TransformNode> children;
	int trans_id;
	int entity_owner;
	std::string ent_name;
	bool isTop = false;
};


class EntityEditor {
public:

	~EntityEditor();
	void init(GraphicsSystem* gs);
	void newEntityWindow(int entity_id);
	void removeEntityWindow();
	void update(float dt);

	

	bool isShowGUI() { return show_entity_; };
	bool overGUI() {
		return is_over_gui_;
	};

	void toggleimGUI() {
		enable_gui_ = !enable_gui_;
		scene_gui = true;
	};
	void closeAll() {
		show_entity_ = false;
		edit_mat = false;
		scene_gui = false;
	}

private:
	//graphics system pointer
	GraphicsSystem* graphics_system_;
	

	//imGUI
	bool enable_gui_ = false;
	bool show_entity_ = false;
	bool add_component_ = false;
	bool edit_mat = false;
	bool scene_gui = true;
	bool is_over_gui_ = false;

	// Background render
	bool show_world_ = false;
	bool show_background_ = false;
	
	lm::vec3 background_color;

	// Selected Entity
	int entity_id = -1;
	bool hasMesh = false;

	// material
	int current_mat = -1;

	// Add component
	const char* adding_components_[3] = { "Collider", "Light", "Mesh" };
	int current_component_ = 0;
	
	
	void updateimGUI_(int texture_id);

	void CameraTargetObject(int entity_id);

	void imGuiRenderElem(TransformNode & trans);

	void sceneGUI();

	void materialGui();

	void addComponentGUI_();

	lm::vec3 rotation;
	lm::vec3 scale;
	lm::vec3 direction;

	//picking
	bool can_fire_picking_ray_ = true;
	int ent_picking_ray_;

	void OneObjectRender(float dt);

	

};