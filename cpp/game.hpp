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
    glm::vec4 color;
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

        const int inner_count = (size + 1) * (size + 1);
        const int edge_count = 4 * size;

        *data = RasterizationData();
        data->vertices = std::vector<VertexData>(inner_count + edge_count);
        data->indices = std::vector<int>(6 * size * size + 6 * edge_count);
        data->par = _par;
        data->start = _course_start_position;
        data->end = _course_putting_green;
        data->mid[0] = _course_midway_points.size() ? _course_midway_points[0] : glm::vec2();
        data->mid[1] = _course_midway_points.size() > 1 ? _course_midway_points[1] : glm::vec2();

        for (int i = 0; i < inner_count; ++i)
        {
            const int ix = i % (size + 1);
            const int iz = i / (size + 1);
            const float x = (float)ix / size;
            const float z = (float)iz / size;
            const float y = 0; // .1 * (sin(x * 4) + sin(z * 4));

            data->vertices[i].position = glm::vec3(x, y, z);
            data->vertices[i].color = glm::vec4(randf(), randf(), randf(), 1);

            if (ix < size && iz < size)
            {
                const int base_index = 6 * (iz * size + ix);

                // triangle 1
                data->indices[base_index + 0] = ix + iz * (size + 1);
                data->indices[base_index + 1] = ix + 1 + iz * (size + 1);
                data->indices[base_index + 2] = ix + (iz + 1) * (size + 1);
                // triangle 2
                data->indices[base_index + 3] = ix + 1 + iz * (size + 1);
                data->indices[base_index + 4] = ix + 1 + (iz + 1) * (size + 1);
                data->indices[base_index + 5] = ix + (iz + 1) * (size + 1);
            }
        }

        const auto index_to_edge = [&](int index, int* const x, int* const z) -> int
        {
            *x = 0;
            *z = 0;

            switch (index / size)
            {
                case 0:
                    *x = index;

                    break;
                case 1:
                    *x = size;
                    *z = index - size;

                    break;
                case 2:
                    *x = 3 * size - index;
                    *z = size;

                    break;
                case 3:
                    *z = edge_count - index;

                    break;
            }

            return *z * (size + 1) + *x;
        };

        for (int i = 0; i < edge_count; ++i)
        {
            float θ = i * 2.f * M_PI / (float)edge_count - M_PI * .1875f;
            float outer_x = .5f + sin(θ) * 1.f;
            float outer_z = .5f - cos(θ) * 1.f;
            int inner_1_x = 0;
            int inner_1_z = 0;
            int inner_2_x = 0;
            int inner_2_z = 0;

            const int inner_index_1 = index_to_edge(i, &inner_1_x, &inner_1_z);
            const int inner_index_2 = index_to_edge(i + 1, &inner_2_x, &inner_2_z);
            const int outer_index_1 = inner_count + i;
            const int outer_index_2 = inner_count + (i + 1) % edge_count;

            data->vertices[outer_index_1].position = glm::vec3(outer_x, 0, outer_z);
            data->vertices[outer_index_1].color = glm::vec4(randf(), randf(), randf(), 1);

            const int base_index = 6 * size * size + 6 * i;

            // triangle 1
            data->indices[base_index + 0] = outer_index_1;
            data->indices[base_index + 1] = inner_index_2;
            data->indices[base_index + 2] = inner_index_1;
            // triangle 2
            data->indices[base_index + 3] = outer_index_1;
            data->indices[base_index + 4] = outer_index_2;
            data->indices[base_index + 5] = inner_index_2;
        }
    }
};
