#version 460 core

precision highp float;


struct SizedVec2 {
    vec2 position;
    float size;
};


uniform float u_time;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec4 u_color_outside_bounds;
uniform vec4 u_color_tee_box;
uniform vec4 u_color_rough;
uniform vec4 u_color_fairway;
uniform vec4 u_color_bunker;
uniform vec4 u_color_putting_green;
uniform vec4 u_color_water;

in vec3 vertex_position;
in vec4 vertex_color;
// TODO : more?

out vec3 fwd_vertex_position;
out vec4 fwd_vertex_color;


void main()
{
    fwd_vertex_position = vertex_position;
    fwd_vertex_color = vertex_color;

    gl_Position = u_projection * u_view * u_model * vec4(vertex_position, 1.0);
}
