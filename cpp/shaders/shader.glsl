#include "common.glsl"


uniform mat4 u_model;
uniform mat4 u_parabola;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_camera_position;
uniform vec3 u_light_position;

uniform vec2 u_dimensions;

uniform struct {
    int par;
    vec2 tee_position;
    vec2 fairway_start_position;
    float tee_size;
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
    vec4 sun;
} u_colors;


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
