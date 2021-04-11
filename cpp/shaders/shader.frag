#include "common.glsl"


precision highp float;

uniform float uniform_time;

in vec4 vertex_position;

layout(location = 0) out vec4 gl_FragColor;


void main()
{
    gl_FragColor = vec4(vertex_position.x, vertex_position.y, (sin(uniform_time) + 1) * .5, 1);
//    gl_FragColor = vec4(1, .5, .2, 1);
}
