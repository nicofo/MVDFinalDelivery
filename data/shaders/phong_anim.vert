#version 330

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec4 a_vertex_weights;
layout(location = 4) in vec4 a_vertex_jointids;


uniform mat4 u_mvp;
uniform mat4 u_vp;
uniform mat4 u_model;
uniform mat4 u_normal_matrix;
uniform vec3 u_cam_pos;

uniform mat4 u_skin_bind_matrix;
const int MAX_JOINTS = 96;
uniform mat4 u_joint_pos_matrices[MAX_JOINTS];
uniform mat4 u_joint_bind_matrices[MAX_JOINTS];

    
out vec2 v_uv;
out vec3 v_normal;
out vec3 v_vertex_world_pos;
out vec3 v_cam_dir;

out float v_color;

void main(){
    
    //uvs
    v_uv = a_uv;
    
    //calculate direction to camera in world space
    v_cam_dir = u_cam_pos - v_vertex_world_pos;
    
    //unweighted position of vertex
    vec4 vertex4 = vec4(a_vertex, 1.0);
    
    //variables to store final position and normal
    vec4 final_vert = vec4(0.0);
    vec3 final_normal = vec3(0.0);
    
    
    //TODO: skeletal animation here

    vec4 vertex4_bsm = u_skin_bind_matrix * vertex4;

    vec3 vertex3_n = mat3(u_skin_bind_matrix) * a_vertex;

    for (int i = 0 ; i < 4; i++) {
        int j_id = int(a_vertex_jointids[i]);
        final_vert +=
        a_vertex_weights[i] * 
        u_joint_pos_matrices[j_id] *
        u_joint_bind_matrices[j_id] * 
        vertex4_bsm;

        final_normal +=
        a_vertex_weights[i] * 
        mat3(u_joint_pos_matrices[j_id]) *
        mat3(u_joint_bind_matrices[j_id]) * 
        vertex3_n;
    }

    //final_normal = (u_normal_matrix * vec4(a_normal, 1.0)).xyz;
    
    //set the final position and normal
    v_vertex_world_pos = final_vert.xyz;
    v_normal = final_normal;
    

    v_color = 0.0f;
    if (int(a_vertex_jointids[0]) == 0) {
        v_color = 1.0f * a_vertex_weights[0];
    }
    //output vertex to NDC
    gl_Position = u_vp * final_vert;
}

