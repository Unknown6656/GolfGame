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

in vec2 tex_coords;

layout(location = 0) out vec4 gl_FragColor;


vec4 fetch_pixelated(vec2 offs)
{
    const vec2 d = u_pixelation_factor / vec2(u_width, u_height);
    const vec2 coord = d * floor(tex_coords / d) + offs;

    return texture(tex_screen, coord);
}

void main()
{
    const ivec2 pixel_coord = ivec2(tex_coords.x * u_width, (1 - tex_coords.y) * u_height);

    if (u_effects == 0)
        gl_FragColor = texture(tex_screen, tex_coords);
    else
    {
        vec2 rgb_offs = vec2(sin(cos(u_time * .1)) * 4 / u_width, 0);

        const vec4 color_g = fetch_pixelated(vec2(0));
        const vec4 color_r = fetch_pixelated(-rgb_offs);
        const vec4 color_b = fetch_pixelated(rgb_offs);
        vec4 color = vec4(color_r.r, color_g.g, color_b.b, color_g.a);

        if (int(u_height * tex_coords.y + sin(.001 * u_time)) % 10 <= 0)
            color *= .8 + .2 * noise3D(vec3(tex_coords * 10, u_time));

        // color = floor(color * COLOR_STEPS) / float(COLOR_STEPS);

        color += noise3D(vec3(tex_coords * vec2(u_width, u_height) * .5, u_time * 5)) * .05;
        color -= length(tex_coords - .5) * .4;
        color *= 1.1;

        gl_FragColor = color;
    }

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
