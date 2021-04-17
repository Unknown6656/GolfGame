#include "common.glsl"


in vec3 vertex_position;
in vec2 vertex_coords;
in int vertex_type;

out vec3 pos;
out vec3 pos_model;
out vec2 coords;
flat out int type;


void main()
{
    pos = vertex_position;
    pos_model = vec3(u_model * vec4(vertex_position, 1.0));
    coords = vertex_coords;
    type = vertex_type;

    mat4 transf = mat4(1.0);
    
    if (type == TYPE_COURSE)
        transf = u_model;
    else if (type == TYPE_PARABOLA)
        transf = u_parabola;

    gl_Position = u_projection * u_view * transf * vec4(vertex_position, 1.0);
}
