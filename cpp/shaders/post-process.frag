#version 460 core

precision highp float;


uniform sampler2D u_screen_texture;

in vec2 tex_coords;

layout(location = 0) out vec4 gl_FragColor;


void main()
{
    gl_FragColor = texture(u_screen_texture, 1 - tex_coords);
}
