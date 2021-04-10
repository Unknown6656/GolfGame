#version 460 core

layout(location = 0) in vec3 aPosition;
out vec4 vertex_position;


void main()
{
    gl_Position = vec4(aPosition, 1.0);
    vertex_position = gl_Position;
}
