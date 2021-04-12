#version 460 core

precision highp float;


struct SizedVec2 {
    vec2 position;
    float size;
};


uniform float uniform_time;
uniform mat4 uniform_model;
uniform mat4 uniform_view;
uniform mat4 uniform_projection;

in vec3 vertex_position;
in vec4 vertex_color;
// TODO : more?

out vec3 fwd_vertex_position;
out vec4 fwd_vertex_color;


void main()
{
    fwd_vertex_position = vertex_position;
    fwd_vertex_color = vertex_color;

    gl_Position = uniform_projection * uniform_view * uniform_model * vec4(vertex_position, 1.0);
}
