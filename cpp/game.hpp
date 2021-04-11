#pragma once

#include "shader.hpp"



enum class Par
{
    Par3 = 0,
    Par4 = 1,
    Par5 = 2
};

struct VertexData
{
    glm::vec3 position;
    // TODO : more?
};

struct SizedVec2
{
    glm::vec2 position;
    float size;
};

struct RasterizationData
{
    std::vector<VertexData> vertices;
    std::vector<int> indices;
    Par par;
    SizedVec2 start;
    glm::vec2 mid[2];
    SizedVec2 end;
};

struct GolfCourse
{
    int _seed;
    Par _par;
    SizedVec2 _course_start_position;
    std::vector<glm::vec2> _course_midway_points;
    SizedVec2 _course_putting_green;


    GolfCourse(const Par par, const int seed)
    {
        std::srand(seed);

        _seed = seed;
        _par = par;

        const float margin = .39f - ((int)par - (int)Par::Par3) * .13f;

        _course_start_position.position = glm::vec2(randf(margin), randf());
        _course_start_position.size = .1f;
        _course_putting_green.position = glm::vec2(randf(margin) + (1 - margin), randf());
        _course_putting_green.size = randf(.15f) + .5f;

        const float φ = std::atan2f(
            _course_putting_green.position.y - _course_start_position.position.y,
            _course_putting_green.position.x - _course_start_position.position.x
        ) + .5f * M_PI;
        const auto lerp = [&](const float f) -> glm::vec2
        {
            return glm::vec2(
                f * _course_start_position.position.x + (1 - f) * _course_putting_green.position.x,
                f * _course_start_position.position.y + (1 - f) * _course_putting_green.position.y
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

    void rasterize(int size, RasterizationData* const data) const
    {
        if (size < 1)
            size = 1;

        *data = RasterizationData();
        data->vertices = std::vector<VertexData>((size + 1) * (size + 1));
        data->indices = std::vector<int>(size * size * 6);
        data->par = _par;
        data->start = _course_start_position;
        data->end = _course_putting_green;
        data->mid[0] = _course_midway_points.size() ? _course_midway_points[0] : glm::vec2();
        data->mid[1] = _course_midway_points.size() > 1 ? _course_midway_points[1] : glm::vec2();

        for (int i = 0; i < data->vertices.size(); ++i)
        {
            const int ix = i % (size + 1);
            const int iz = i / (size + 1);
            const float x = (float)ix / size;
            const float z = (float)iz / size;
            const float y = .0f;

            // data->vertices[i].position = glm::vec3(x, y, z);
            data->vertices[i].position = glm::vec3(x, z, y);

            if (ix < size && iz < size)
            {
                data->indices[(ix + iz * size) * 6 + 0] = ix + iz * (size + 1);
                data->indices[(ix + iz * size) * 6 + 1] =
                data->indices[(ix + iz * size) * 6 + 3] = ix + 1 + iz * (size + 1);
                data->indices[(ix + iz * size) * 6 + 4] = ix + 1 + (iz + 1) * (size + 1);
                data->indices[(ix + iz * size) * 6 + 2] =
                data->indices[(ix + iz * size) * 6 + 5] = ix + (iz + 1) * (size + 1);
            }
        }
    }
};
