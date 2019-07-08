#include "EntityEditor.h"
#include "extern.h"
#include "Parsers.h"
#include "shaders_default.h"
#include "DebugSystem.h"


EntityEditor::~EntityEditor()
{
}

void EntityEditor::init(GraphicsSystem* gs) {
	graphics_system_ = gs;
}

void EntityEditor::newEntityWindow(int entity_id) {
	this->entity_id = entity_id;
	show_entity_ = true;
	scale = lm::vec3(1, 1, 1);
	rotation = lm::vec3(0, 0, 0);
	hasMesh = ECS.hasComponent<Mesh>(entity_id);
	show_world_ = true;
	show_background_ = true;
	if (ECS.hasComponent<Light>(entity_id)) {
		Light& light = ECS.getComponentFromEntity<Light>(entity_id);
		direction = light.direction;
	}
	
}

void EntityEditor::removeEntityWindow() {
	this->entity_id = -1;
	show_entity_ = false;
}




//called once per frame
void EntityEditor::update(float dt) {

	if (!enable_gui_) return;
	

	//imGUI
	glBindVertexArray(0);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	if (show_entity_) {
		int texture_id = 0;
		if (!show_world_ && hasMesh) {
			if (show_background_)
				texture_id = graphics_system_->renderEntityToTexture(entity_id);
			else 
				texture_id = graphics_system_->renderEntityToTexture(entity_id, 
					lm::vec4(background_color.x, background_color.y, background_color.z, 1)
				);
		}
		updateimGUI_(texture_id);
	}
	if (add_component_ && show_entity_) {
		addComponentGUI_();
	}
	if (edit_mat) {
		materialGui();
	}
	if (scene_gui) {
		sceneGUI();
	}
	ImGuiIO &io = ImGui::GetIO();
	is_over_gui_ = io.WantCaptureMouse == 1;

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
}	

void EntityEditor::addComponentGUI_() {
	static const char* current_item;
	current_item = adding_components_[current_component_];

	ImGui::Begin("Add component", &add_component_);
	{
		if (ImGui::BeginCombo("Component", current_item))
		{
			for (int n = 0; n < IM_ARRAYSIZE(adding_components_); n++)
			{
				bool is_selected = (current_component_ == n);
				if (ImGui::Selectable(adding_components_[n], is_selected)) {
					current_component_ = n;
					
				}
				if (is_selected) {

					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Add") && add_component_) {
			add_component_ = false;
			switch (current_component_) {

			case 0:
				if (!ECS.hasComponent<Collider>(entity_id)) {
					std::cout << "Collider" << std::endl;
					ECS.createComponentForEntity<Collider>(entity_id);
				}
				else {
					std::cout << "Collider already exists" << std::endl;
				}
				break;
			case 1:
				if (!ECS.hasComponent<Light>(entity_id)) {
					std::cout << "Light" << std::endl;
					ECS.createComponentForEntity<Light>(entity_id);
				}
				else {
					std::cout << "Tag already exists" << std::endl;
				}
				break;
			case 2:
				if (!ECS.hasComponent<Mesh>(entity_id)) {
					std::cout << "Mesh" << std::endl;
					ECS.createComponentForEntity<Mesh>(entity_id);
				}
				else {
					std::cout << "Mesh already exists" << std::endl;
				}
				break;
			default:
				std::cout << "Nope" << std::endl;
			}
			int current_component_ = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			add_component_ = false;
			int current_component_ = 0;
		}
		
	}
	ImGui::End();
}

void EntityEditor::updateimGUI_(int texture_id) {

		if (entity_id <= -1) return;
		//open window
		std::string title;
		title.append("Entity: ");
		title.append(ECS.getEntityName(entity_id));
		ImGui::SetNextWindowBgAlpha(1.0);
		ImGui::Begin(title.c_str(), &show_entity_);


		if (ImGui::Button("Focus"))
		{
			Transform& transform = ECS.getComponentFromEntity<Transform>(entity_id);
			Camera& cam = ECS.getComponentInArray<Camera>(ECS.main_camera);
			cam.forward = transform.position() - cam.position;
			float distance = cam.position.distance(transform.position());
			if (distance > 10.0f) {
				float t = 1.0f - (10.0f / distance);
				cam.position = cam.position + cam.forward * t;
				Transform& transformCam = ECS.getComponentFromEntity<Transform>(cam.owner);
				transformCam.position(cam.position);
			}
			cam.update();
		}
		ImGui::SameLine();

		if (ImGui::Button("Destroy Entity"))
		{
			ECS.removeEntity(entity_id);
			show_entity_ = false;
			entity_id = -1;
			ImGui::End();
			return;
		}
		ImGui::NextColumn();

		//region Transforms
		if (ECS.hasComponent<Transform>(entity_id)) {

			if (ImGui::TreeNode("Transforms")) {
				Transform& transform = ECS.getComponentFromEntity<Transform>(entity_id);
				
				// position
				lm::vec3 pos = transform.position();
				float pos_array[3] = { pos.x, pos.y, pos.z };
				ImGui::DragFloat3("Position", pos_array, 0.05f);
				transform.position(pos_array[0], pos_array[1], pos_array[2]);
				

				// rotation
				float rot_array[3] = { rotation.x, rotation.y, rotation.z };
				ImGui::DragFloat3("Rotation", rot_array);
				transform.rotateLocal((rotation.x - rot_array[0]) * DEG2RAD, lm::vec3(1, 0, 0));
				transform.rotateLocal((rotation.y - rot_array[1]) * DEG2RAD, lm::vec3(0, 1, 0));
				transform.rotateLocal((rotation.z - rot_array[2]) * DEG2RAD, lm::vec3(0, 0, 1));
				rotation = lm::vec3(rot_array[0], rot_array[1], rot_array[2]);

				// scale
				float scale_array[3] = { scale.x, scale.y, scale.z };
				ImGui::DragFloat3("Scale", scale_array, 0.01f, 0.0f);
				lm::vec3 tmpScale = lm::vec3(scale_array[0] - scale.x + 1, scale_array[1] - scale.y + 1, scale_array[2] - scale.z + 1);
				scale = lm::vec3(scale_array[0], scale_array[1], scale_array[2]);
				transform.scale(tmpScale);

				if (ECS.hasComponent<Light>(entity_id)) {
					Light& l = ECS.getComponentFromEntity<Light>(entity_id);
					l.position = transform.position();
					l.update();
					graphics_system_->needUpdateLights = true;
				}

				ImGui::TreePop();
			}
		}
		// endregion

		//region Mesh
		if (ECS.hasComponent<Mesh>(entity_id)) {
			if (ImGui::TreeNode("Mesh")) {
				Mesh& mesh = ECS.getComponentFromEntity<Mesh>(entity_id);
				if (mesh.material > -1) {
					static const char* current_item = graphics_system_->getMaterial(mesh.material).name.c_str();
					
					std::vector<Material>& materials = graphics_system_->getMaterials();
					if (ImGui::BeginCombo("Material", current_item))
					{
						int n = 0;
						for (auto& mat: materials)
						{
							bool is_selected = (mesh.material == n);
							if (ImGui::Selectable(mat.name.c_str(), is_selected)) {
								mesh.material = n;

							}
							if (is_selected) {

								ImGui::SetItemDefaultFocus();
							}
							n++;
						}
						ImGui::EndCombo();
					}
					/*if (ImGui::Button("Create material"))
					{
						mesh.material = graphics_system_->createMaterial();
						Material& mat = graphics_system_->getMaterial(mesh.material);
						mat.name = "new material";
					}*/
					
					if (mesh.material > -1) {
						//ImGui::SameLine();
						if (ImGui::Button("Edit material"))
						{
							current_mat = mesh.material;
							edit_mat = true;
						}
					}

				}

				ImGui::TreePop();
			}
		}
		// endregion

		//region Light
		if (ECS.hasComponent<Light>(entity_id)) {
			if (ImGui::TreeNode("Light")) {;
				Light& light = ECS.getComponentFromEntity<Light>(entity_id);

				// direction
				float dir_array[3] = { direction.x, direction.y, direction.z };
				ImGui::DragFloat3("direction", dir_array, 0.05f);
				direction = lm::vec3(dir_array[0], dir_array[1], dir_array[2]);
				light.direction = direction.normalize();
				light.forward = light.direction;

			

				// color
				lm::vec3 color = light.color;
				float color_array[3] = { color.x, color.y, color.z };
				ImGui::DragFloat3("color", color_array, 0.01f, 0.0f, 1.0f);
				light.color = lm::vec3(color_array[0], color_array[1], color_array[2]);

				// type
				ImGui::Text("Light Type:");
				ImGui::Indent(16.0f);
				static int e = light.type;
				ImGui::RadioButton("Directionl", &e, 0); ImGui::SameLine();
				ImGui::RadioButton("Point", &e, 1); ImGui::SameLine();
				ImGui::RadioButton("Spot", &e, 2);
				ImGui::Unindent(16.0f);
				light.type = static_cast<LightType>(e);

				// Attenuation
				ImGui::Text("Atenuation:");
				ImGui::Indent(16.0f);
				ImGui::DragFloat("linear", &light.linear_att, 0.01f, 0.0f);
				ImGui::DragFloat("quadratic", &light.quadratic_att, 0.01f, 0.0f);
				ImGui::Unindent(16.0f);

				// Spot
				if (light.type == LightTypeSpot) {
					ImGui::Text("Spot:");
					ImGui::Indent(16.0f);
					ImGui::DragFloat("inner", &light.spot_inner, 0.01f, 0.0f, light.spot_outer); 
					ImGui::DragFloat("outer", &light.spot_outer, 0.01f, light.spot_inner);
					ImGui::Unindent(16.0f);
				}
				bool cast_shadow = !!light.cast_shadow;
				ImGui::Checkbox("Cast shadow", &cast_shadow);
				light.cast_shadow = cast_shadow ? 1 : 0;
				if (cast_shadow) {
					ImGui::DragInt("shadow resolution ", &light.resolution, 1.0f, 8);
				}

				light.calculateRadius();
				light.update();
				graphics_system_->needUpdateLights = true;

				ImGui::TreePop();
			}
		}
		// endregion

		ImGui::Spacing();

		if (ImGui::Button("Add Component")) {
			add_component_ = true;
		}

		ImGui::Separator();

		//region viewport options
		if (hasMesh) {
			if (ImGui::TreeNode("Viewport options")) {
				ImGui::Checkbox("Show world", &show_world_);
				if (!show_world_) {
					ImGui::Checkbox("Show background", &show_background_);
					if (!show_background_) {
						float color_array[3] = { background_color.x, background_color.y, background_color.z };
						ImGui::DragFloat3("Background", color_array, 0.05f, 0.0f, 1.0f);
						background_color.x = color_array[0];
						background_color.y = color_array[1];
						background_color.z = color_array[2];
					}
				}
				ImGui::TreePop();
			}
		}

		ImGui::End();
		

}

void EntityEditor::CameraTargetObject(int entity_id) {
	Transform& transform = ECS.getComponentFromEntity<Transform>(entity_id);
	Camera& cam = ECS.getComponentInArray<Camera>(ECS.main_camera);
	cam.forward = transform.position() - cam.position;
	float distance = cam.position.distance(transform.position());
	if (distance > 10.0f) {
		float t = 1.0f - (10.0f / distance);
		cam.position = cam.position + cam.forward * t;
		Transform& transformCam = ECS.getComponentFromEntity<Transform>(cam.owner);
		transformCam.position(cam.position);
	}
	cam.update();
}

// recursive function to render a transform node in imGUI
void EntityEditor::imGuiRenderElem(TransformNode& trans) {
	auto& ent = ECS.entities[trans.entity_owner];
	if (ImGui::TreeNode(ent.name.c_str())) {
		Transform& transform = ECS.getComponentFromEntity<Transform>(ent.name);

		if (ImGui::Button("Go to"))
		{
			CameraTargetObject(transform.owner);
		}
		if (ImGui::Button("Edit"))
		{
			newEntityWindow(trans.entity_owner);
			CameraTargetObject(transform.owner);
		}
		ImGui::TreePop();
	}
}

void EntityEditor::sceneGUI() {

		//get input
		ImGuiIO &io = ImGui::GetIO();

		//open window
		ImGui::SetNextWindowBgAlpha(1.0);
		ImGui::Begin("Scene", &scene_gui);

		//sliders for blend shapes

//        auto& test_blendshapes = ECS.getAllComponents<BlendShapes>()[0];
//        for (size_t i = 0; i < test_blendshapes.blend_weights.size(); i++) {
//            ImGui::SliderFloat(test_blendshapes.blend_names[i].c_str(),
//                               &(test_blendshapes.blend_weights[i]),
//                               0.0f, 1.0f,
//                               "%.3f");
//        }

		//Tell imGUI to display variables of the camera
		//get camera and its transform
		Camera& cam = ECS.getComponentInArray<Camera>(ECS.main_camera);
		Transform& cam_transform = ECS.getComponentFromEntity<Transform>(cam.owner);

		//Create an unfoldable tree node called 'Camera'
		if (ImGui::TreeNode("Camera")) {
			//create temporary arrays with position and direction data
			float cam_pos_array[3] = { cam.position.x, cam.position.y, cam.position.z };
			float cam_dir_array[3] = { cam.forward.x, cam.forward.y, cam.forward.z };

			//create imGUI components that allow us to change the values when click-dragging
			ImGui::DragFloat3("Position", cam_pos_array);
			ImGui::DragFloat3("Direction", cam_dir_array);

			//use values of temporary arrays to set real values (in case user changes)
			cam.position = lm::vec3(cam_pos_array[0], cam_pos_array[1], cam_pos_array[2]);
			cam_transform.position(cam.position);
			cam.forward = lm::vec3(cam_dir_array[0], cam_dir_array[1], cam_dir_array[2]).normalize();
			ImGui::TreePop();
		}

		//create a tree of TransformNodes objects (defined in DebugSystem.h)
		//which represents the current scene graph

		// 1) create a temporary array with ALL transforms
		std::vector<TransformNode> transform_nodes;
		std::vector<int> entities_id;
		auto& all_transforms = ECS.getAllComponents<Transform>();
		for (size_t i = 0; i < all_transforms.size(); i++) {
			TransformNode tn;
			tn.trans_id = (int)i;
			tn.entity_owner = all_transforms[i].owner;
			tn.ent_name = ECS.entities[tn.entity_owner].name;
			if (all_transforms[i].parent == -1)
				tn.isTop = true;
			transform_nodes.push_back(tn);
		}

		// 2) traverse array to assign children to their parents
		for (size_t i = 0; i < transform_nodes.size(); i++) {
			int parent = all_transforms[i].parent;
			if (parent != -1) {
				transform_nodes[parent].children.push_back(transform_nodes[i]);
			}
		}

		// 3) create a new array with only top level nodes of transform tree
		std::vector<TransformNode> transform_topnodes;
		for (size_t i = 0; i < transform_nodes.size(); i++) {
			if (transform_nodes[i].isTop)
				transform_topnodes.push_back(transform_nodes[i]);
		}

		//create 2 imGUI columns, first contains transform tree
		//second contains selected item from picking

		//draw all the nodes
		for (auto& trans : transform_topnodes) {
			//this is a recursive function (defined above)
			//which draws a transform node (and its children)
			//using imGUI
			imGuiRenderElem(trans);
		}


		ImGui::End();
	
}

void EntityEditor::materialGui() {

	// Start the Dear ImGui frame



	ImGuiIO &io = ImGui::GetIO();


	//open window
	ImGui::SetNextWindowBgAlpha(1.0);
	ImGui::Begin("Material: ", &edit_mat);

	if (current_mat < 0) return;
	Material& material = graphics_system_->getMaterial(current_mat);
	char char_array[64];

	// Name 
	strcpy(char_array, material.name.c_str());
	ImGui::InputText("", char_array, IM_ARRAYSIZE(char_array));
	material.name = std::string(char_array);

	// Basic
	if (ImGui::TreeNode("Basic material")) {

		// Diffuse
		float dif_array[3] = { material.diffuse.x, material.diffuse.y, material.diffuse.z };
		ImGui::DragFloat3("diffuse", dif_array, 0.05f, 0.0f, 2.0f);
		material.diffuse = lm::vec3(dif_array[0], dif_array[1], dif_array[2]);

		// specular
		float spc_array[3] = { material.specular.x, material.specular.y, material.specular.z };
		ImGui::DragFloat3("specular", spc_array, 0.05f, 0.0f, 2.0f);
		material.specular = lm::vec3(spc_array[0], spc_array[1], spc_array[2]);

		// ambient
		float amb_array[3] = { material.ambient.x, material.ambient.y, material.ambient.z };
		ImGui::DragFloat3("ambient", amb_array, 0.05f, 0.0f, 2.0f);
		material.ambient = lm::vec3(amb_array[0], amb_array[1], amb_array[2]);

		//gloss
		float gl_float = material.specular_gloss;
		ImGui::DragFloat("specular gloss", &gl_float, 0.05f, 0.0f);
		material.specular_gloss = gl_float;

		ImGui::TreePop();
	}


	if (ImGui::TreeNode("Textures")) {

		// uv scale
		float uv_array[2] = { material.uv_scale.x, material.uv_scale.y };
		ImGui::DragFloat2("UV scale", uv_array, 0.05f, 0.0f);
		material.uv_scale = lm::vec2(uv_array[0], uv_array[1]);


		// Diffuse
		ImGui::Text("Diffuse map 1:");
		if (material.diffuse_map >= 0) {
			ImGui::Image((ImTextureID)material.diffuse_map, ImVec2(100, 100));
		}

		ImGui::Text("Diffuse map 2:");
		if (material.diffuse_map_2 >= 0) {
			ImGui::Image((ImTextureID)material.diffuse_map_2, ImVec2(100, 100));
		}

		ImGui::Text("Diffuse map 3:");
		if (material.diffuse_map_3 >= 0) {
			ImGui::Image((ImTextureID)material.diffuse_map_3, ImVec2(100, 100));
		}

		ImGui::Text("Normal map:");
		if (material.normal_map >= 0) {
			ImGui::Image((ImTextureID)material.normal_map, ImVec2(100, 100));
		}

		ImGui::Text("Specular map:");
		if (material.specular_map >= 0) {
			ImGui::Image((ImTextureID)material.specular_map, ImVec2(100, 100));
		}

		ImGui::Text("Transparency map:");
		if (material.transparency_map >= 0) {
			ImGui::Image((ImTextureID)material.transparency_map, ImVec2(100, 100));
		}

		ImGui::Text("Noise map:");
		if (material.noise_map >= 0) {
			ImGui::Image((ImTextureID)material.noise_map, ImVec2(100, 100));
		}

		ImGui::Text("Noise map:");
		if (material.cube_map >= 0) {
			ImGui::Image((ImTextureID)material.cube_map, ImVec2(100, 100));
		}
		
		ImGui::TreePop();
	}

	
	ImGui::End();
	
}