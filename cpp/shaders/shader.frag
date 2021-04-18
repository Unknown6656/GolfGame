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

vec4 main_parabola()
{
    // check if   1-4(x-.5)^2 == y
    const float x = coords.x;
    float y = 2 * (x - .5);

    y *= y;
    y = 1 - y - coords.y * (1 + PARABOLA_THICKNESS);

    return abs(y) <= PARABOLA_THICKNESS ? lerp(x, vec4(.7, 0, 0, .3), vec4(1, 0, 0, .7)) : vec4(0.);
}

vec4 main_course()
{
    const vec2 opos = displaced_pos(3, .05);
    const bool is_outside_field = opos.x + EPS < 0 || opos.x - EPS > u_dimensions.x || opos.y + EPS < 0 || opos.y - EPS > u_dimensions.y;
    vec4 color;

    if (length(pos.xz - u_golf_course.end_position) < u_golf_course.end_size)
    {
        color = u_colors.putting_green;
        color += HATCHET_INTENSITY * (int(pos.x * HATCHET_X + pos.z * HATCHET_Y + HATCHET_OFFS) % 2 - int(pos.z * HATCHET_X - pos.x * HATCHET_Y + HATCHET_OFFS) % 2);
    }
    else if (length(pos.xz - u_golf_course.tee_position) < u_golf_course.tee_size)
        color = u_colors.tee_box;
    else
    {
        float dist_to_fairway;

        if (u_golf_course.par == PAR_3)
            dist_to_fairway = distance_line_point(u_golf_course.fairway_start_position, u_golf_course.end_position, pos.xz);
        else if (u_golf_course.par == PAR_4)
            dist_to_fairway = min(
                distance_line_point(u_golf_course.fairway_start_position, u_golf_course.mid1_position, pos.xz),
                distance_line_point(u_golf_course.mid1_position, u_golf_course.end_position, pos.xz)
            );
        else if (u_golf_course.par == PAR_5)
            dist_to_fairway = min(min(
                distance_line_point(u_golf_course.fairway_start_position, u_golf_course.mid1_position, pos.xz),
                distance_line_point(u_golf_course.mid1_position, u_golf_course.mid2_position, pos.xz)),
                distance_line_point(u_golf_course.mid2_position, u_golf_course.end_position, pos.xz)
            );

        dist_to_fairway += noise2D(pos.xy * 5) * .1;

        const float fairway_size = lerp(
            (pos.x - u_golf_course.tee_position.x) / (u_golf_course.end_position.x - u_golf_course.tee_position.x),
            u_golf_course.tee_size,
            u_golf_course.end_size
        ) + .15;

        if (dist_to_fairway < fairway_size)
        {
            color = u_colors.fairway;
            color += HATCHET_INTENSITY * (int(pos.x * HATCHET_X + pos.z * HATCHET_Y + HATCHET_OFFS) % 2);
        }
        else if (is_outside_field)
            color = u_colors.outside_bounds;
        else
            color = u_colors.rough;
    }

    // diffuse shading
    color += max(dot(vec3(0, 1, 0), normalize(u_light_position - pos_model)), 0) * .2 * u_colors.sun;

    return color;
}

void main()
{
    if (type == TYPE_COURSE)
    {
        const vec2 para_start = (u_parabola * vec4(0, 0, 0, 1)).xz;
        const vec2 para_end = (u_parabola * vec4(1, 0, 0, 1)).xz;

        if (distance(pos_model.xz, para_start) < PARABOLA_THICKNESS)
            gl_FragColor = vec4(.5, 0, 0, 1);
        else if (distance(pos_model.xz, para_end) < PARABOLA_THICKNESS)
            gl_FragColor = vec4(1, 0, 0, 1);
        else
            gl_FragColor = main_course();
    }
    else if (type == TYPE_PARABOLA)
        gl_FragColor = main_parabola();
    else
        gl_FragColor = vec4(1, 0, 1, 1);
}
