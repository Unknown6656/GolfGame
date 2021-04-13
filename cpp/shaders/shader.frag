#version 460 core

precision highp float;


uniform float u_time;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec2 u_dimensions;

uniform vec4 u_color_outside_bounds;
uniform vec4 u_color_tee_box;
uniform vec4 u_color_rough;
uniform vec4 u_color_fairway;
uniform vec4 u_color_bunker;
uniform vec4 u_color_putting_green;
uniform vec4 u_color_water;

in vec3 fwd_vertex_position;
in vec2 fwd_vertex_coords;
in vec4 fwd_vertex_color;

layout(location = 0) out vec4 gl_FragColor;


const float EPS = .00001f;


void main()
{
    const float x = fwd_vertex_position.x;
    const float y = fwd_vertex_position.z;
    const float u = fwd_vertex_coords.x;
    const float v = fwd_vertex_coords.y;
    const float elevation = fwd_vertex_position.y;

    const bool is_in_field = !(x + EPS < 0 || x - EPS > u_dimensions.x || y + EPS < 0 || y - EPS > u_dimensions.y);

    if (is_in_field)
        gl_FragColor = vec4(u, v, (sin(u_time) + 1) * .5, 1);
    else
        gl_FragColor = u_color_outside_bounds;
}
