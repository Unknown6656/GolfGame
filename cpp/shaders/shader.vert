#version 460 core

precision highp float;

layout(location = 0) in vec3 position;

out vec4 vertex_position;


void main()
{
    gl_Position = vec4(position, 1.0);
    vertex_position = gl_Position;
}
