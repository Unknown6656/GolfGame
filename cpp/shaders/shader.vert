#include "common.glsl"


in vec3 vertex_position;
in vec2 vertex_coords;
in vec4 vertex_color;
// TODO : more?

out vec3 pos;
out vec2 coords;
out vec4 color;


void main()
{
    pos = vertex_position;
    coords = vertex_coords;
    color = vertex_color;

    gl_Position = u_projection * u_view * u_model * vec4(vertex_position, 1.0);
}
