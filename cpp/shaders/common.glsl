#version 460 core

precision highp float;

#define PI 3.14159265358979323846264338327950288419716939937
#define TAU 6.28318530717958647692528676655900576839433879875
#define EPS 0.00001
#define PAR3 0
#define PAR4 1
#define PAR5 2

#ifndef saturate
    #define saturate(v) clamp(v, 0., 1.)
#endif

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
    const vec2 i = floor(position);
    const vec2 f = fract(position);
    const float a = random(i);
    const float b = random(i + vec2(1.0, 0.0));
    const float c = random(i + vec2(0.0, 1.0));
    const float d = random(i + vec2(1.0, 1.0));
    const vec2 u = smoothstep(0.0, 1.0, f);

    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

vec3 hue2rgb(float hue)
{
    hue = fract(hue);

    return saturate(vec3(
        abs(hue * 6. - 3.) - 1.,
        2. - abs(hue * 6. - 2.),
        2. - abs(hue * 6. - 4.)
    ));
}
