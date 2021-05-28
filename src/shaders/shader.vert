#include "shader.glsl"


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
    pos_model = vec3(u_model * vec4(vertex_position, 1));
    coords = vertex_coords;
    type = vertex_type;

    mat4 transf;

    if (type == TYPE_COURSE || type == TYPE_TREE)
        transf = u_model;
    else if (type == TYPE_PARABOLA)
        transf = u_parabola;
    else if (type == TYPE_PLAYER)
        transf = u_player;
    else if (type == TYPE_FLAGPOLE)
        transf = u_flagpole * mat4(mat3(.5));
    else
        transf = mat4(1);

    gl_Position = u_projection * u_view * transf * vec4(vertex_position, 1);
}
