#include "font.glsl"


in vec2 coords;

layout(location = 0) out vec4 gl_FragColor;


void main()
{
    gl_FragColor = text_color * texture(text, coords).r;
}
