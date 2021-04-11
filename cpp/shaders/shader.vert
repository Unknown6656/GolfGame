#include "common.glsl"


precision highp float;

in vec3 position;

out vec4 vertex_position;


void main()
{
    gl_Position = vec4(position, 1.0);
    vertex_position = gl_Position;
}
