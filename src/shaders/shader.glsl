#include "common.glsl"


#define SURFACE_TYPE_UNDEFINED 0
#define SURFACE_TYPE_TEEBOX 1
#define SURFACE_TYPE_TEEPOINT 2
#define SURFACE_TYPE_PUTTINGGREEN 3
#define SURFACE_TYPE_PUTTINGHOLE 4
#define SURFACE_TYPE_FAIRWAY 5
#define SURFACE_TYPE_ROUGH 6
#define SURFACE_TYPE_BUNKER 7
#define SURFACE_TYPE_WATER 8
#define SURFACE_TYPE_OUTSIDECOURSE 9

#define PLAYER_STATE_WAITING 0

#define PARABOLA_THICKNESS .02

#define TREE_WIDTH 85
#define TREE_HEIGHT 130


uniform mat4 u_model;
uniform mat4 u_parabola;
uniform mat4 u_player;
uniform mat4 u_flagpole;
uniform float u_parabola_height;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_camera_position;
uniform vec3 u_light_position;
uniform float u_ball_position;
uniform int u_player_state;

uniform vec2 u_dimensions;

uniform sampler2D tex_surface;
uniform sampler2D tex_flagpole;
uniform vec2 tex_flagpole_size;
uniform sampler2D tex_player;
uniform vec2 tex_player_size;
uniform sampler2D tex_trees;
uniform vec2 tex_trees_size;

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

uniform struct {
    vec2 position;
    float point_size;
    float area_size;
} u_putting_green;


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
