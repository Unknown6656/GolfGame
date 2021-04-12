#version 460 core

precision highp float;


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

in vec3 fwd_vertex_position;
in vec4 fwd_vertex_color;

layout(location = 0) out vec4 gl_FragColor;


void main()
{
    const bool is_in_field = fwd_vertex_position.x >= 0 && fwd_vertex_position.x <= 1.0
                          && fwd_vertex_position.y >= 0 && fwd_vertex_position.y <= 1.0;

    if (is_in_field)
        gl_FragColor = vec4(fwd_vertex_position.x, fwd_vertex_position.z, (sin(u_time) + 1) * .5, 1);
    else
        gl_FragColor = vec4(92 / 255.0, 140 / 255.0, 70/255.0, 1.0);
}
