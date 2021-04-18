#include "common.glsl"

#define COLOR_STEPS 8


uniform sampler2D u_screen_texture;
uniform float u_pixelation_factor;
uniform int u_width;
uniform int u_height;

in vec2 tex_coords;

layout(location = 0) out vec4 gl_FragColor;


vec4 fetch_pixelated(vec2 offs)
{
    const vec2 d = u_pixelation_factor / vec2(u_width, u_height);
    const vec2 coord = d * floor(tex_coords / d) + offs;

    return texture(u_screen_texture, coord);
}

void main()
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
