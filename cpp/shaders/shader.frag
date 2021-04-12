#version 460 core

precision highp float;


uniform float uniform_time;
uniform mat4 uniform_model;
uniform mat4 uniform_view;
uniform mat4 uniform_projection;

in vec3 fwd_vertex_position;
in vec4 fwd_vertex_color;

layout(location = 0) out vec4 gl_FragColor;


void main()
{
    gl_FragColor = vec4(fwd_vertex_position.x, fwd_vertex_position.z, (sin(uniform_time) + 1) * .5, 1);
//    gl_FragColor = fwd_vertex_color;
}
