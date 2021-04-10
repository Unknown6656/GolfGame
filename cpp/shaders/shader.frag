#version 460 core

precision highp float;

uniform float uniform_time;
in vec4 vertex_position;

layout(location = 0) out vec4 gl_FragColor;


void main()
{
    float t;
    modf(uniform_time, t);

    gl_FragColor = vec4(vertex_position.x, vertex_position.y, sin(uniform_time), 1.0f);
    gl_FragColor = vec4(t, 0.0f, sin(uniform_time), 1.0f);
//    gl_FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
