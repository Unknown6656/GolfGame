#include "common.glsl"


in vec3 pos;
in vec2 coords;
in vec4 color;

layout(location = 0) out vec4 gl_FragColor;



vec2 displaced_pos(float frequency, float amount)
{
    const float offs = noise2D(pos.xz * 2 * frequency) * amount;
    const float dir = noise2D(pos.xz * frequency) * TAU;

    return vec2(pos.x + cos(dir) * offs, pos.z + sin(dir) * offs);
}

void main()
{
    const vec2 opos = displaced_pos(3, .05);
    const bool is_outside_field = opos.x + EPS < 0 || opos.x - EPS > u_dimensions.x || opos.y + EPS < 0 || opos.y - EPS > u_dimensions.y;

    if (is_outside_field)
        gl_FragColor = u_colors.outside_bounds;
    else if (length(pos.xz - u_golf_course.end_position) < u_golf_course.end_size)
        gl_FragColor = u_colors.putting_green;
    else if (length(pos.xz - u_golf_course.start_position) < u_golf_course.start_size)
        gl_FragColor = u_colors.tee_box;
    else
    {
        if (u_golf_course.par == PAR3)
            ;
        else if (u_golf_course.par == PAR4)
            ;
        else if (u_golf_course.par == PAR5)
            ;




        gl_FragColor = u_colors.rough;




    }

//    gl_FragColor = vec4(noise2D(pos.xz * 8.0));

//    gl_FragColor = vec4(u_golf_course.end_size);
}
