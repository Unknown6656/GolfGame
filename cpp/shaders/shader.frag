#include "common.glsl"


in vec3 pos;
in vec2 coords;
in vec4 color;

layout(location = 0) out vec4 gl_FragColor;


const float EPS = .00001f;


void main()
{
    float 

    float x = pos;
    float y = ;




    const bool is_outside_field = pos.x + EPS < 0 || pos.x - EPS > u_dimensions.x || pos.z + EPS < 0 || pos.z - EPS > u_dimensions.y;

    if (is_outside_field)
        gl_FragColor = u_colors.outside_bounds;
    else if (length(pos.xz - u_golf_course.end_position) < u_golf_course.end_size)
        gl_FragColor = u_colors.putting_green;
    else if (length(pos.xz - u_golf_course.start_position) < u_golf_course.start_size)
        gl_FragColor = u_colors.tee_box;
    else
    {
        gl_FragColor = u_colors.rough;
    }

    gl_FragColor = vec4(noise2D(gl_FragCoord.xy));
    gl_FragColor = vec4(noise2D(pos.xz * 8.0));

//    gl_FragColor = vec4(u_golf_course.end_size);
}
