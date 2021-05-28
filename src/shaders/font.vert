#include "font.glsl"


in vec2 glyph_position;
in vec2 glyph_coords;

out vec2 coords;


void main()
{
    coords = vec2(glyph_coords.x, 1 - glyph_coords.y);

    const vec2 gpos = .5 * (glyph_position + 1);

    if (use_projection)
        gl_Position = projection * vec4(gpos, 0, 1);
    else
    {
        const vec2 spos = pos - .5 * screen_size;

        gl_Position = vec4(2 * (gpos * size + spos) / screen_size, 0, 1);
    }
}
