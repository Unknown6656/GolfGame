#pragma once

#define _USE_MATH_DEFINES

#include <vector>
#include <random>
#include <math.h>
#include <cmath>

#include <glm/glm.hpp>


inline float randf() noexcept
{
    return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

inline float randf(const float max) noexcept
{
    return randf() * max;
}


enum class Par
{
    Par3 = 0,
    Par4 = 1,
    Par5 = 2
};

struct GolfCourse
{
    int _seed;
    Par _par;
    glm::vec2 _course_start_position;
    std::vector<glm::vec2> _course_midway_points;
    struct
    {
        glm::vec2 position;
        float size;
    } _course_putting_green;


    GolfCourse(const Par par, const int seed)
    {
        std::srand(seed);

        _seed = seed;
        _par = par;

        const float margin = .39f - ((int)par - (int)Par::Par3) * .13f;

        _course_start_position = glm::vec2(randf(margin), randf());
        _course_putting_green.position = glm::vec2(randf(margin) + (1 - margin), randf());
        _course_putting_green.size = randf(.15f);

        const float φ = std::atan2f(
            _course_putting_green.position.y - _course_start_position.y,
            _course_putting_green.position.x - _course_start_position.x
        ) + .5f * M_PI;
        const auto lerp = [&](const float f) -> glm::vec2
        {
            return glm::vec2(
                f * _course_start_position.x + (1 - f) * _course_putting_green.position.x,
                f * _course_start_position.y + (1 - f) * _course_putting_green.position.y
            );
        };

        if (par == Par::Par4)
        {
            const float mid_pos = randf(.6f) + .2f;
            const float mid_dev = randf(.5f) + .25f;
            glm::vec2 mid = lerp(mid_pos);

            mid += glm::vec2(std::cosf(φ) * mid_dev, std::sinf(φ) * mid_dev);

            _course_midway_points = { mid };
        }
        else if (par == Par::Par4)
        {
            const float mid_dev_1 = randf(.5f) + .25f;
            const float mid_dev_2 = randf(.5f) + .25f;
            glm::vec2 mid_1 = lerp(.3f);
            glm::vec2 mid_2 = lerp(.6f);

            mid_1 += glm::vec2(std::cosf(φ) * mid_dev_1, std::sinf(φ) * mid_dev_1);
            mid_2 += glm::vec2(std::cosf(φ) * mid_dev_2, std::sinf(φ) * mid_dev_2);

            _course_midway_points = { mid_1, mid_2 };
        }
        else
            _course_midway_points = std::vector<glm::vec2>();
    }
};
