#version 460 core

precision highp float;

#define PI 3.14159265358979323846264338327950288419716939937
#define TAU 6.28318530717958647692528676655900576839433879875
#define EPS 0.00001
#define PAR_3 0
#define PAR_4 1
#define PAR_5 2
#define TYPE_COURSE 0
#define TYPE_PARABOLA 1

#ifndef saturate
    #define saturate(v) clamp(v, 0., 1.)
#endif

uniform float u_time;
uniform mat4 u_model;
uniform mat4 u_parabola;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_camera_position;
uniform vec3 u_light_position;

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

float random(in vec3 position)
{
    position = fract(position * .3183099 + .1) * 17;

    return fract(position.x * position.y * position.z * (position.x + position.y + position.z));
}

float noise3D(in vec3 position)
{
    vec3 i = floor(position);
    vec3 f = fract(position);

    f *= f * (3 - 2 * f);

    return mix(mix(mix(random(i + vec3(0, 0, 0)), 
                       random(i + vec3(1, 0, 0)), f.x),
                   mix(random(i + vec3(0, 1, 0)), 
                       random(i + vec3(1, 1, 0)), f.x), f.y),
               mix(mix(random(i + vec3(0, 0, 1)), 
                       random(i + vec3(1, 0, 1)), f.x),
                   mix(random(i + vec3(0, 1, 1)), 
                       random(i + vec3(1, 1, 1)), f.x), f.y), f.z);
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

float distance_line_point(in vec2 line_start, in vec2 line_end, in vec2 point)
{
    const float l = distance(line_start, line_end);

    if (l <= 0)
        return distance(point, line_start);

    const float t = saturate(dot(point - line_start, line_end - line_start) / (l * l));
    const vec2 proj = line_start + t * (line_end - line_start);

    return distance(point, proj);
}
