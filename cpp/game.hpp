#pragma once

#include "common.hpp"


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
    Player = 2,
    Flagpole = 3,
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
    float area_size;
    float point_size;
};

enum class SurfaceType
    : unsigned char
{
    UNDEFINED = 0,
    TeeBox = 1,
    TeePoint = 2,
    Rough = 3,
    Fairway = 4,
    OutsideCourse = 5,
    PuttingGreen = 6,
    PuttingHole = 7,
    Bunker = 8,
    Water = 9,
};

struct RasterizationData
{
    std::vector<VertexData> vertices;
    std::vector<int> indices;
    glm::vec2 dimensions;
    float aspect_ratio;
    Par par;
    SizedVec2 tee;
    glm::vec2 start;
    glm::vec2 mid[2];
    SizedVec2 end;
    struct
    {
        int width;
        int height;
        std::vector<SurfaceType> pixels;
    } surface;


    inline SurfaceType get_ball_position(const glm::vec2& ball_position) const noexcept
    {
        const int x = ball_position.x / dimensions.x * surface.width;
        const int y = ball_position.y / dimensions.y * surface.height;

        if (x < 0 || y < 0 || x >= surface.width || y > surface.height)
            return SurfaceType::OutsideCourse;
        else
            return surface.pixels[y * surface.width + x];
    }
};

struct Noise
{
    glm::vec2 seed;
    float scale;


    Noise() noexcept
        : seed(12.9898f, 78.233f)
        , scale(43758.5453123f)
    {
    }

    inline float random(const glm::vec2& position) const noexcept
    {
        const float val = sin(glm::dot(position, seed)) * scale;

        return val - floor(val);
    }

    inline float noise2d(const glm::vec2& position) const noexcept
    {
        const glm::vec2 i = glm::floor(position);
        const glm::vec2 f = glm::fract(position);
        const float a = random(i);
        const float b = random(i + glm::vec2(1.0, 0.0));
        const float c = random(i + glm::vec2(0.0, 1.0));
        const float d = random(i + glm::vec2(1.0, 1.0));
        const glm::vec2 u = glm::smoothstep(0.f, 1.f, f);

        return glm::mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
    }
};

struct GolfCourse
{
    int _seed;
    Par _par;
    float _length;
    SizedVec2 _course_start_position;
    glm::vec2 _fairway_start_position;
    std::vector<glm::vec2> _course_midway_points;
    SizedVec2 _course_putting_green;
    Noise _noise;


    GolfCourse(const Par par, const float length, const int seed)
    {
        std::srand(seed);

        _par = par;
        _seed = seed;
        _length = length;
        _noise = Noise();
        _noise.seed -= seed;

        const float margin = .39f - ((int)par - (int)Par::Par3) * .13f;

        _course_start_position.point_size = .001f;
        _course_start_position.area_size = .1f;
        _course_start_position.position = glm::vec2(
            randf(margin) + _course_start_position.area_size,
            randf(1.f - 2.f * _course_start_position.area_size) + _course_start_position.area_size
        );
        _course_putting_green.point_size = .001f;
        _course_putting_green.area_size = randf(.1f) + .07f;
        _course_putting_green.position = glm::vec2(
            randf(margin) + (length - margin - _course_putting_green.area_size),
            randf(1.f - 2.f * _course_putting_green.area_size) + _course_putting_green.area_size
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

        const float fsp = (_course_start_position.area_size + .2) * 2;

        _fairway_start_position = (1 - fsp) * _course_start_position.position + fsp * (par == Par::Par3 ? _course_putting_green.position : _course_midway_points[0]);
    }

    void rasterize(const int subdivisions, const int texture_height, RasterizationData* const data) const
    {
#define RADIUS 2.5f

        const int size_y = std::max(1, subdivisions);
        const int size_x = size_y * _length;
        const float ratio = (float)size_x / size_y;
        const int inner_count = (size_x + 1) * (size_y + 1);
        const int edge_count = 2 * size_x + 2 * size_y;

        *data = RasterizationData();
        data->aspect_ratio = ratio;
        data->dimensions = glm::vec2(ratio, 1.f);
        data->vertices = std::vector<VertexData>(inner_count + edge_count);
        data->indices = std::vector<int>(6 * size_x * size_y + 6 * edge_count);
        data->par = _par;
        data->tee = _course_start_position;
        data->start = _fairway_start_position;
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



        const int texture_width = texture_height * ratio;

        data->surface.width = texture_width;
        data->surface.height = texture_height;
        data->surface.pixels = std::vector<SurfaceType>(texture_height * texture_width);

        Concurrency::parallel_for(size_t(0), size_t(texture_height * texture_width), [&](size_t i)
        {
            const int x = i % texture_width;
            const int y = i / texture_width;

            constexpr float FREQ = 3.f;
            constexpr float AMOUNT = .05f;
            const glm::vec2 position(
                (float)x / (float)texture_height,
                (float)y / (float)texture_height
            );
            const float displacement_dir = _noise.noise2d(position * FREQ) * M_PI * 2.f;
            const float displacement_offs = _noise.noise2d(position * 2.f * FREQ) * AMOUNT;
            const glm::vec2 displaced = position + displacement_offs * glm::vec2(cos(displacement_dir), sin(displacement_dir));
            const bool outside = displaced.x <= 0 || displaced.y <= 0 || displaced.x >= ratio || displaced.y >= 1;
            SurfaceType& type = data->surface.pixels[y * texture_width + x];

            if (glm::distance(position, data->end.position) < data->end.point_size)
                type = SurfaceType::PuttingHole;
            else if (glm::distance(position, data->end.position) < data->end.area_size)
                type = SurfaceType::PuttingGreen;
            else if (glm::distance(position, data->tee.position) < data->tee.point_size)
                type = SurfaceType::TeePoint;
            else if (glm::distance(position, data->tee.position) < data->tee.area_size)
                type = SurfaceType::TeeBox;
            else
            {
                // TODO : water
                // TODO : bunker

                float distance_to_fairway = 0.f;

                if (_par == Par::Par3)
                    distance_to_fairway = distance_line_point(data->start, data->end.position, position);
                else if (_par == Par::Par4)
                    distance_to_fairway = std::min(
                        distance_line_point(data->start, data->mid[0], position),
                        distance_line_point(data->mid[0], data->end.position, position)
                    );
                else if (_par == Par::Par5)
                    distance_to_fairway = std::min(std::min(
                        distance_line_point(data->start, data->mid[0], position),
                        distance_line_point(data->mid[0], data->mid[1], position)),
                        distance_line_point(data->mid[1], data->end.position, position)
                    );

                distance_to_fairway += _noise.noise2d(position * 5.f) * .1;

                const float fairway_size_factor = (position.x - data->tee.position.x) / (data->end.position.x - data->tee.position.x);
                const float fairway_size = glm::mix(data->tee.area_size, data->end.area_size, fairway_size_factor) + .15f;

                if (distance_to_fairway < fairway_size)
                    type = SurfaceType::Fairway;
                else if (outside)
                    type = SurfaceType::OutsideCourse;
                else
                    type = SurfaceType::Rough;
            }
        });

#undef RADIUS
    }
};

enum class GolfClubType
{
    UNKNOWN = 0,
    Driver = 1,
    Wood1 = 1,
    Wood2 = 2,
    Iron3 = 3,
    Iron4 = 4,
    Iron5 = 5,
    Iron6 = 6,
    Iron7 = 7,
    Iron8 = 8,
    Iron9 = 9,
    PitchingWedge = 10,
    SandWedge = 11,
    Putter = 12,
};
