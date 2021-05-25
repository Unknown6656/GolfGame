#include "shader.glsl"


in vec3 pos;
in vec3 pos_model;
in vec2 coords;
flat in int type;

layout(location = 0) out vec4 gl_FragColor;



vec4 main_parabola()
{
    // check if   1-4(x-.5)^2 == y
    const float x = 1 - pow(2 * (coords.x - .5), 2);
    const float y = PARABOLA_THICKNESS + coords.y / (1 - 2 * PARABOLA_THICKNESS);

    return abs(x - y) <= .3 * PARABOLA_THICKNESS / u_parabola_height ? lerp(coords.x, vec4(.7, 0, 0, .3), vec4(1, 0, 0, .7)) : vec4(0);
}

vec4 main_course()
{
    // const float displaced_offs = noise2D(pos.xz * 40) * .02;
    // const float displaced_dir = noise2D(pos.xz * 20) * TAU;
    // const vec2 displaced_coords = coords + mix(
    //     vec2(cos(displaced_dir), sin(displaced_dir)) * displaced_offs / u_dimensions,
    //     vec2(0),
    //     noise2D(pos.xz * 30)
    // );
    const int type = int(texture2D(tex_surface, coords).x * 255);
    vec4 color;

    if (type == SURFACE_TYPE_TEEBOX)
        color = u_colors.tee_box;
    else if (type == SURFACE_TYPE_TEEPOINT)
        color = vec4(0, 0, 0, 1);
    else if (type == SURFACE_TYPE_ROUGH)
        color = u_colors.rough;
    else if (type == SURFACE_TYPE_FAIRWAY)
    {
        color = u_colors.fairway;
        color += HATCHET_INTENSITY * (int(pos.x * HATCHET_X + pos.z * HATCHET_Y + HATCHET_OFFS) % 2);
    }
    else if (type == SURFACE_TYPE_PUTTINGGREEN)
    {
        color = u_colors.putting_green;
        color += HATCHET_INTENSITY * (int(pos.x * HATCHET_X + pos.z * HATCHET_Y + HATCHET_OFFS) % 2 - int(pos.z * HATCHET_X - pos.x * HATCHET_Y + HATCHET_OFFS) % 2);
    }
    else if (type == SURFACE_TYPE_PUTTINGHOLE)
        color = vec4(0, 0, 0, 1);
    else if (type == SURFACE_TYPE_BUNKER)
        color = u_colors.water; // TODO: water movement
    else if (type == SURFACE_TYPE_WATER)
        color = u_colors.bunker; // TODO: noise
    else
        // SURFACE_TYPE_OUTSIDECOURSE
        // SURFACE_TYPE_UNDEFINED
        color = u_colors.outside_bounds;


    // diffuse shading
    const float sun_steps = .05;
    const vec3 sun_position = u_effects == 0 ? pos_model : sun_steps * floor(pos_model / sun_steps);
    const float sun_intensity = pow(max(dot(vec3(0, 1, 0), normalize(u_light_position - sun_position)), 0), 5) * .5;

    if (u_effects == 0)
        color += sun_intensity * u_colors.sun;
    else
        color += sun_steps * .3 * floor(sun_intensity / sun_steps * 2) * u_colors.sun;

    return color;
}

void main()
{
    if (type == TYPE_COURSE)
    {
        const vec2 para_start = (u_parabola * vec4(0, 0, 0, 1)).xz;
        const vec2 para_end = (u_parabola * vec4(1, 0, 0, 1)).xz;

        if (distance(pos_model.xz, para_start) < PARABOLA_THICKNESS * .25)
            gl_FragColor = vec4(.5, 0, 0, 1);
        else if (distance(pos_model.xz, para_end) < PARABOLA_THICKNESS * .25)
            gl_FragColor = vec4(1, 0, 0, 1);
        else
            gl_FragColor = main_course();
    }
    else if (type == TYPE_PARABOLA)
        gl_FragColor = main_parabola();
    else
        gl_FragColor = vec4(1, 0, 1, 1);
}
