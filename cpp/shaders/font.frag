#include "common.glsl"


uniform mat4 projection;
uniform vec4 text_color;
uniform sampler2D text;

in vec2 tex_coords;

layout(location = 0) out vec4 gl_FragColor;


void main()
{
    gl_FragColor = text_color * texture(text, tex_coords).r;
}
