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

#define PARABOLA_THICKNESS .01

#define HATCHET_X 20
#define HATCHET_Y 12
#define HATCHET_OFFS 3
#define HATCHET_INTENSITY .03


#ifndef saturate
    #define saturate(v) clamp(v, 0., 1.)
#endif

#ifndef lerp
    #define lerp(f, x, y) ((1 - (f)) * (x) + (f) * (y))
#endif


uniform float u_time;
