#include "common.glsl"


in vec3 pos;
in vec3 pos_model;
in vec2 coords;
flat in int type;

layout(location = 0) out vec4 gl_FragColor;


vec2 displaced_pos(float frequency, float amount)
{
    const float offs = noise2D(pos.xz * 2 * frequency) * amount;
    const float dir = noise2D(pos.xz * frequency) * TAU;

    return vec2(pos.x + cos(dir) * offs, pos.z + sin(dir) * offs);
}

vec4 main_course()
{
    const vec2 opos = displaced_pos(3, .05);
    const bool is_outside_field = opos.x + EPS < 0 || opos.x - EPS > u_dimensions.x || opos.y + EPS < 0 || opos.y - EPS > u_dimensions.y;
    vec4 color;

    if (is_outside_field)
        color = u_colors.outside_bounds;
    else if (length(pos.xz - u_golf_course.end_position) < u_golf_course.end_size)
        color = u_colors.putting_green;
    else if (length(pos.xz - u_golf_course.start_position) < u_golf_course.start_size)
        color = u_colors.tee_box;
    else
    {
        float dist_to_fairway;

        if (u_golf_course.par == PAR_3)
            dist_to_fairway = distance_line_point(u_golf_course.start_position, u_golf_course.end_position, pos.xz);
        else if (u_golf_course.par == PAR_4)
            dist_to_fairway = min(
                distance_line_point(u_golf_course.start_position, u_golf_course.mid1_position, pos.xz),
                distance_line_point(u_golf_course.mid1_position, u_golf_course.end_position, pos.xz)
            );
        else if (u_golf_course.par == PAR_5)
            dist_to_fairway = min(min(
                distance_line_point(u_golf_course.start_position, u_golf_course.mid1_position, pos.xz),
                distance_line_point(u_golf_course.mid1_position, u_golf_course.mid2_position, pos.xz)),
                distance_line_point(u_golf_course.mid2_position, u_golf_course.end_position, pos.xz)
            );

        color = dist_to_fairway < max(u_golf_course.start_size, u_golf_course.end_size) + .1 ? u_colors.fairway : u_colors.rough;
    }

    // shading
    color *= max(dot(vec3(0, 1, 0), normalize(u_light_position - pos_model)), 0);

    return color;
}

vec4 main_parabola()
{
    bool inside = false;



    inside = length(coords) > .5; // TODO
    inside = true;


    return inside ? vec4(1., 0., 0., 1.) : vec4(0.);
}

void main()
{
    if (type == TYPE_COURSE)
        gl_FragColor = main_course();
    else if (type == TYPE_PARABOLA)
        gl_FragColor = main_parabola();
}
