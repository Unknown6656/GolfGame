#pragma once

#include "shader.hpp"


enum class Par
{
    Par3 = 0,
    Par4 = 1,
    Par5 = 2
};

enum class VertexType
{
    Course = 0,
    Parabola = 1,
};

struct VertexData
{
    glm::vec3 position;
    glm::vec2 coords;
    VertexType type;

    VertexData(const glm::vec3 pos, const glm::vec2 coord, const VertexType type) noexcept
        : position(pos)
        , coords(coord)
        , type(type)
    {
    }

    VertexData() noexcept
        : VertexData(glm::vec3(), glm::vec2(), VertexType::Course)
    {
    }

    std::string to_string() const noexcept
    {
        return format(
            "[%10f,%10f,%10f|%10f,%10f|%i]",
            position.x,
            position.y,
            position.z,
            coords.x,
            coords.y,
            (int)type
        );
    }

    OSTREAM_OPERATOR(VertexData);
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
    glm::vec2 dimensions;
    Par par;
    SizedVec2 start;
    glm::vec2 mid[2];
    SizedVec2 end;
};

struct GolfCourse
{
    int _seed;
    Par _par;
    float _length;
    SizedVec2 _course_start_position;
    std::vector<glm::vec2> _course_midway_points;
    SizedVec2 _course_putting_green;


    GolfCourse(const Par par, const float length, const int seed)
    {
        std::srand(seed);

        _par = par;
        _seed = seed;
        _length = length;

        const float margin = .39f - ((int)par - (int)Par::Par3) * .13f;

        _course_start_position.size = .1f;
        _course_start_position.position = glm::vec2(
            randf(margin) + _course_start_position.size,
            randf(1.f - 2.f * _course_start_position.size) + _course_start_position.size
        );
        _course_putting_green.size = randf(.15f) + .1f;
        _course_putting_green.position = glm::vec2(
            randf(margin) + (length - margin - _course_putting_green.size),
            randf(1.f - 2.f * _course_putting_green.size) + _course_putting_green.size
        );

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

    void rasterize(const int subdivisions, RasterizationData* const data) const
    {
#define RADIUS 1.5f

        const int size_y = std::max(1, subdivisions);
        const int size_x = size_y * _length;
        const float ratio = (float)size_x / size_y;
        const int inner_count = (size_x + 1) * (size_y + 1);
        const int edge_count = 2 * size_x + 2 * size_y;

        *data = RasterizationData();
        data->dimensions = glm::vec2(ratio, 1.f);
        data->vertices = std::vector<VertexData>(inner_count + edge_count);
        data->indices = std::vector<int>(6 * size_x * size_y + 6 * edge_count);
        data->par = _par;
        data->start = _course_start_position;
        data->end = _course_putting_green;
        data->mid[0] = _course_midway_points.size() ? _course_midway_points[0] : glm::vec2();
        data->mid[1] = _course_midway_points.size() > 1 ? _course_midway_points[1] : glm::vec2();

        for (int i = 0; i < inner_count; ++i)
        {
            const int ix = i % (size_x + 1);
            const int iy = i / (size_x + 1);
            const float x = (float)ix / size_y;
            const float y = (float)iy / size_y;
            const float elevation = 0; // .1 * (sin(x * 4) + sin(z * 4));

            data->vertices[i].position = glm::vec3(x, elevation, y);
            data->vertices[i].type = VertexType::Course;

            if (ix < size_x && iy < size_y)
            {
                const int base_index = 6 * (iy * size_x + ix);

                // triangle 1
                data->indices[base_index + 0] = ix + iy * (size_x + 1);
                data->indices[base_index + 1] = ix + 1 + iy * (size_x + 1);
                data->indices[base_index + 2] = ix + (iy + 1) * (size_x + 1);
                // triangle 2
                data->indices[base_index + 3] = ix + 1 + iy * (size_x + 1);
                data->indices[base_index + 4] = ix + 1 + (iy + 1) * (size_x + 1);
                data->indices[base_index + 5] = ix + (iy + 1) * (size_x + 1);
            }
        }

        const auto index_to_edge = [&](int index, int* const x, int* const y) -> int
        {
            *x = 0;
            *y = 0;

            if (index < size_x)
                *x = index;
            else if (index < size_x + size_y)
            {
                *x = size_x;
                *y = index - size_x;
            }
            else if (index < 2 * size_x + size_y)
            {
                *x = 2 * size_x + size_y - index;
                *y = size_y;
            }
            else if (index < edge_count)
                *y = edge_count - index;

            return *y * (size_x + 1) + *x;
        };

        for (int i = 0; i < edge_count; ++i)
        {
            int inner_1_x = 0;
            int inner_1_y = 0;
            int inner_2_x = 0;
            int inner_2_y = 0;

            const int inner_index_1 = index_to_edge(i, &inner_1_x, &inner_1_y);
            const int inner_index_2 = index_to_edge(i + 1, &inner_2_x, &inner_2_y);
            const int outer_index_1 = inner_count + i;
            const int outer_index_2 = inner_count + (i + 1) % edge_count;

            glm::vec2 center(ratio * .5f, .5f);
            glm::vec2 edge_mid = .5f * (data->vertices[inner_index_1].position.xz() +
                                        data->vertices[inner_index_2].position.xz());
            glm::vec2 outer_pos = glm::normalize(edge_mid - center) * RADIUS;

            outer_pos.x *= ratio * .5f;
            outer_pos += center;

            data->vertices[outer_index_1].position = glm::vec3(outer_pos.x, 0, outer_pos.y);
            data->vertices[outer_index_1].type = VertexType::Course;

            const int base_index = 6 * size_x * size_y + 6 * i;

            // triangle 1
            data->indices[base_index + 0] = outer_index_1;
            data->indices[base_index + 1] = inner_index_2;
            data->indices[base_index + 2] = inner_index_1;
            // triangle 2
            data->indices[base_index + 3] = outer_index_1;
            data->indices[base_index + 4] = outer_index_2;
            data->indices[base_index + 5] = inner_index_2;
        }

        for (VertexData& vertex : data->vertices)
            vertex.coords = vertex.position.xz / glm::vec2(ratio, 1.f);

#undef RADIUS
    }
};
