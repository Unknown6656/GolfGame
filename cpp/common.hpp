#pragma once

#define _USE_MATH_DEFINES
#define GLEW_STATIC

#define VERSION_MAJ 4
#define VERSION_MIN 6

#define F_NEAR .01f
#define F_FAR 1000.f


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdio>
#include <windows.h>
#include <vector>
#include <random>
#include <math.h>
#include <cmath>

// #include <GL/glew.h>
// #include <GL/wglew.h>
// #include <GL/glu.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <KHR/khrplatform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define nameof(name) #name


inline float randf() noexcept
{
    return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

inline float randf(const float max) noexcept
{
    return randf() * max;
}

inline glm::vec3 from_rgb(unsigned int rgb) noexcept
{
    rgb &= 0x00ffffff;

    return glm::vec3(
        ((rgb & 0x00ff0000) >> 16) / 255.0,
        ((rgb & 0x0000ff00) >> 8) / 255.0,
        (rgb & 0x000000ff) / 255.0
    );
}

inline glm::vec4 from_argb(const unsigned int argb)
{
    return glm::vec4(
        ((argb & 0xff000000) >> 24) / 255.0,
        ((argb & 0x00ff0000) >> 16) / 255.0,
        ((argb & 0x0000ff00) >> 8) / 255.0,
        (argb & 0x000000ff) / 255.0
    );
}
