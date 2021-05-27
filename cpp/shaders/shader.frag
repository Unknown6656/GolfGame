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
    const vec2 ball_diff = coords - vec2(u_ball_position, 1 - pow(2 * (u_ball_position - .5), 2) + PARABOLA_THICKNESS);
    const float ball_dist = sqrt(ball_diff.x * ball_diff.x + ball_diff.y * ball_diff.y * u_parabola_height);

    if (ball_dist <= PARABOLA_THICKNESS * 1.2)
        return vec4(1);
    else if (u_animating == 0 && abs(x - y) <= .3 * PARABOLA_THICKNESS / u_parabola_height)
        return lerp(coords.x, vec4(.7, 0, 0, .3), vec4(1, 0, 0, .7));
    else
        return vec4(0);
}

vec4 main_player()
{
    vec2 texture_coords = (coords + vec2(u_player_state, 0)) * vec2(tex_player_size.y / tex_player_size.x, 1);

    if (u_player_state == PLAYER_STATE_WAITING)
        texture_coords.y = mix(texture_coords.y, .5 * (1 - pow(1 - 2 * texture_coords.y, 3)), sin(u_time * 2) * .1);

    return texture2D(tex_player, texture_coords);
}

vec4 main_course()
{
    const int type = int(texture2D(tex_surface, coords).x * 255);
    vec4 color = vec4(1, 0, 1, 1);

    if (type == SURFACE_TYPE_TEEPOINT)
        color = vec4(0, 0, 0, 1);
    else if (type == SURFACE_TYPE_TEEBOX)
        color = u_colors.tee_box;
    else if (type == SURFACE_TYPE_ROUGH)
        color = u_colors.rough;
    else if (type == SURFACE_TYPE_FAIRWAY)
    {
        color = u_colors.fairway;
        color += HATCHET_INTENSITY * (int(pos.x * HATCHET_X + pos.z * HATCHET_Y + HATCHET_OFFS) % 2);
    }
    else if (type == SURFACE_TYPE_PUTTINGHOLE || distance(pos.xz, u_putting_green.position) < u_putting_green.point_size)
        color = vec4(0, 0, 0, 1);
    else if (type == SURFACE_TYPE_PUTTINGGREEN)
    {
        color = u_colors.putting_green;
        color += HATCHET_INTENSITY * (int(pos.x * HATCHET_X + pos.z * HATCHET_Y + HATCHET_OFFS) % 2 - int(pos.z * HATCHET_X - pos.x * HATCHET_Y + HATCHET_OFFS) % 2);
    }
    else if (type == SURFACE_TYPE_BUNKER)
        color = u_colors.water; // TODO: water movement
    else if (type == SURFACE_TYPE_WATER)
        color = u_colors.bunker; // TODO: noise
    else
    {
        const float displaced_offs = noise2D(pos_model.xz * 6) * .2;
        const float displaced_dir = noise2D(pos_model.xz * 3) * TAU;
        const vec2 displaced = pos_model.xz + vec2(cos(displaced_dir), sin(displaced_dir)) * displaced_offs / u_dimensions;
        bvec2 comp = greaterThan(abs(displaced), u_dimensions * .5 + .2);

        if (comp.x || comp.y)
            color = u_colors.outside_bounds;
        else
            color = u_colors.rough;
    }

    // diffuse shading
    const float sun_steps = .05;
    const vec3 sun_position = u_effects == 0 ? pos_model : sun_steps * floor(pos_model / sun_steps);
    const float sun_intensity = pow(max(dot(vec3(0, 1, 0), normalize(u_light_position - sun_position)), 0), 5) * .5 + sin(u_time * .1) * .02;

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

        if (u_animating == 0 && distance(pos_model.xz, para_start) < PARABOLA_THICKNESS * .25)
            gl_FragColor = vec4(.5, 0, 0, 1);
        else if (u_animating == 0 && distance(pos_model.xz, para_end) < PARABOLA_THICKNESS * .25)
            gl_FragColor = vec4(1, 0, 0, 1);
        else if (u_ball_position >= .99 && distance(pos_model.xz, para_end) < PARABOLA_THICKNESS * .2)
            gl_FragColor = vec4(1);
        else
            gl_FragColor = main_course();
    }
    else if (type == TYPE_PARABOLA)
        gl_FragColor = main_parabola();
    else if (type == TYPE_PLAYER)
        gl_FragColor = main_player();
    else if (type == TYPE_FLAGPOLE)
        gl_FragColor = texture2D(tex_flagpole, coords);
    else
        gl_FragColor = vec4(1, 0, 1, 1);

    if (gl_FragColor.a < .05)
        discard;
}
