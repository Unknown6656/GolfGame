#include "common.glsl"


in vec2 screen_position;
in vec2 screen_coords;

out vec2 tex_coords;


void main()
{
    tex_coords = screen_coords;
    gl_Position = vec4(screen_position, 0, 1);
}  