#include "common.glsl"

#define COLOR_STEPS 8

#define UI_LEFT 10
#define UI_BOTTOM 10
#define UI_WIDTH 300
#define UI_HEIGHT 98


uniform sampler2D tex_screen;
uniform sampler2D tex_clubs;
uniform vec2 tex_clubs_size;

uniform float u_pixelation_factor;
uniform int u_width;
uniform int u_height;

uniform int u_selected_club;
uniform float u_player_strength;
uniform int u_has_won;

in vec2 tex_coords;

layout(location = 0) out vec4 gl_FragColor;


vec4 blur1D(vec2 uv, vec2 direction)
{
    const vec2 off1 = vec2(1.411764705882353) * direction;
    const vec2 off2 = vec2(3.2941176470588234) * direction;
    const vec2 off3 = vec2(5.176470588235294) * direction;
    vec4 color = vec4(0.0);

    color += texture(tex_screen, uv) * 0.1964825501511404;
    color += texture(tex_screen, uv + off1) * 0.2969069646728344;
    color += texture(tex_screen, uv - off1) * 0.2969069646728344;
    color += texture(tex_screen, uv + off2) * 0.09447039785044732;
    color += texture(tex_screen, uv - off2) * 0.09447039785044732;
    color += texture(tex_screen, uv + off3) * 0.010381362401148057;
    color += texture(tex_screen, uv - off3) * 0.010381362401148057;

    return color;
}

vec4 blur2D(vec2 uv, float radius)
{
    vec4 color = vec4(0.0);
    const float count = 9;

    for (int i = 0; i < count; ++i)
    {
        const float angle = i * TAU / count;

        color += blur1D(uv, vec2(radius * sin(angle) / u_width, radius * cos(angle) / u_height));
    }

    return color / count;
}

vec4 fetch_pixelated(vec2 offs)
{
    const vec2 d = u_pixelation_factor / vec2(u_width, u_height);
    const vec2 coord = d * floor(tex_coords / d) + offs;

    return texture(tex_screen, coord);
}

vec4 fetch_and_distort(vec2 uv)
{
    const vec2 rgb_offs = vec2(sin(cos(u_time * .1)) * 4 / u_width, 0);
    vec4 color;

    if (u_has_won != 0)
        color = blur2D(tex_coords, 8);
    else
    {
        const vec4 color_g = fetch_pixelated(vec2(0));
        const vec4 color_r = fetch_pixelated(-rgb_offs);
        const vec4 color_b = fetch_pixelated(rgb_offs);

        color = vec4(color_r.r, color_g.g, color_b.b, color_g.a);
    }

    if (int(u_height * uv.y + sin(.001 * u_time)) % 10 <= 0)
        color *= .8 + .2 * noise3D(vec3(uv * 10, u_time));

    // color = floor(color * COLOR_STEPS) / float(COLOR_STEPS);

    color += noise3D(vec3(uv * vec2(u_width, u_height) * .5, u_time * 5)) * .05;
    color -= length(uv - .5) * .4;
    color *= 1.1;

    return color;
}

void main()
{
    const ivec2 pixel_coord = ivec2(tex_coords.x * u_width, (1 - tex_coords.y) * u_height);

    if (u_effects == 0)
        gl_FragColor = texture(tex_screen, tex_coords);
    else
        gl_FragColor = fetch_and_distort(tex_coords);

    if (u_animating == 0 &&
        pixel_coord.x >= UI_LEFT &&
        pixel_coord.x < UI_LEFT + UI_WIDTH &&
        pixel_coord.y >= u_height - UI_BOTTOM - UI_HEIGHT &&
        pixel_coord.y < u_height - UI_BOTTOM)
    {
        const ivec2 ui_pixel_coord = ivec2(pixel_coord.x - UI_LEFT, pixel_coord.y - u_height + UI_BOTTOM + UI_HEIGHT);
        vec4 ui_color;

        const float club_size = (tex_clubs_size.y + 1) * 2;

        if (ui_pixel_coord.x < club_size && ui_pixel_coord.y < club_size)
            ui_color = texture(tex_clubs, vec2((ui_pixel_coord.x + (club_size - 2) * u_selected_club) / tex_clubs_size.x * .5, ui_pixel_coord.y / club_size));
        else if (ui_pixel_coord.x > club_size + 2 && ui_pixel_coord.x < club_size + 30 && ui_pixel_coord.y < club_size)
        {
            if (ui_pixel_coord.y / club_size > 1 - u_player_strength)
                ui_color = vec4(1 - u_player_strength, u_player_strength, 0, 1);
        }
        else
        {
            // TODO : more UI stuff like compass (?)
        }

        gl_FragColor = mix(gl_FragColor, ui_color, ui_color.a);
    }
}
