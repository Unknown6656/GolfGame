#version 460 core

precision highp float;


uniform float u_time;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec2 u_dimensions;

uniform struct {
    int par;
    vec2 start_position;
    float start_size;
    vec2 mid1_position;
    vec2 mid2_position;
    vec2 end_position;
    float end_size;
} u_golf_course;

uniform struct {
    vec4 outside_bounds;
    vec4 tee_box;
    vec4 rough;
    vec4 fairway;
    vec4 bunker;
    vec4 putting_green;
    vec4 water;
} u_colors;


float random(in vec2 position)
{
    return fract(sin(dot(position, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise2D(in vec2 position)
{
    vec2 i = floor(position);
    vec2 f = fract(position);
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = smoothstep(0.0, 1.0, f);

    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}
