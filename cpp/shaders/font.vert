#include "common.glsl"


uniform mat4 projection;
uniform vec4 text_color;
uniform sampler2D text;

in vec4 glyph_poscoord;

out vec2 tex_coords;


void main()
{
    gl_Position = projection * vec4(glyph_poscoord.xy, 0, 1);
    tex_coords = glyph_poscoord.zw;
}
