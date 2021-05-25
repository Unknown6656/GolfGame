#pragma once

#define _USE_MATH_DEFINES
#define GLM_SWIZZLE
#define GLEW_STATIC

#define VERSION_MAJ 4
#define VERSION_MIN 6

#define F_NEAR .01f
#define F_FAR 1000.f

#define PIXELATION_FACTOR 6.f


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <regex>
#include <cstdio>
#include <windows.h>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <math.h>
#include <cmath>
#include <map>
#include <ppl.h>

// #include <GL/glew.h>
// #include <GL/wglew.h>
// #include <GL/glu.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <KHR/khrplatform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H


#define nameof(name) #name

#define OSTREAM_OPERATOR(x) friend inline std::ostream& operator<<(std::ostream& os, const x& value) { os << value.to_string(); return os; }


inline float randf() noexcept
{
    return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

inline float randf(const float max) noexcept
{
    return randf() * max;
}

inline float map(const float x, const float from_min, const float from_max, const float to_min, const float to_max) noexcept
{
    return (x - from_min) / (to_min - from_min) * (to_max - from_max) + from_max;
}

inline glm::vec4 from_argb(const unsigned int argb)
{
    return glm::vec4(
        ((argb & 0x00ff0000) >> 16) / 255.0,
        ((argb & 0x0000ff00) >> 8) / 255.0,
        (argb & 0x000000ff) / 255.0,
        ((argb & 0xff000000) >> 24) / 255.0
    );
}

inline glm::vec3 from_rgb(unsigned int rgb) noexcept
{
    return from_argb(rgb & 0x00ffffff).xyz();
}

inline float distance_line_point(const glm::vec2 line_start, const glm::vec2 line_end, const glm::vec2 point) noexcept
{
    const float l = glm::distance(line_start, line_end);

    if (l <= 0)
        return glm::distance(point, line_start);

    const float t = std::max(0.f, std::min(1.f, (glm::dot(point - line_start, line_end - line_start) / (l * l))));
    const glm::vec2 proj = line_start + t * (line_end - line_start);

    return glm::distance(point, proj);
}

inline void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }));
}

inline void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }).base(), s.end());
}

inline void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

template <typename T>
inline T smooothstep(const T& x)
{
    const T clamped = glm::min(glm::max(T(0.f), x), T(1.f));

    return clamped * clamped * (3.f - 2.f * clamped);
}

template <typename T>
inline T lerp(const T& x, const T& y, float factor)
{   
    return y * factor + x * (1.f - factor);
}

template <typename ...Args>
inline std::string format(const std::string& format, Args&& ...args)
{
    const int size = std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...);
    std::string output(size + 1, '\0');

    sprintf_s(&output[0], size + 1, format.c_str(), std::forward<Args>(args)...);

    return output;
}

namespace std
{
    template<class BidirIt, class Traits, class CharT, class UnaryFunction>
    std::basic_string<CharT> regex_replace_f(BidirIt first, BidirIt last, const std::basic_regex<CharT, Traits>& re, UnaryFunction f)
    {
        std::basic_string<CharT> s;
        typename std::match_results<BidirIt>::difference_type positionOfLastMatch = 0;
        auto endOfLastMatch = first;

        auto callback = [&](const std::match_results<BidirIt>& match)
        {
            auto positionOfThisMatch = match.position(0);
            auto diff = positionOfThisMatch - positionOfLastMatch;
            auto startOfThisMatch = endOfLastMatch;

            std::advance(startOfThisMatch, diff);

            s.append(endOfLastMatch, startOfThisMatch);
            s.append(f(match));

            auto lengthOfMatch = match.length(0);

            positionOfLastMatch = positionOfThisMatch + lengthOfMatch;
            endOfLastMatch = startOfThisMatch;

            std::advance(endOfLastMatch, lengthOfMatch);
        };

        std::regex_iterator<BidirIt> begin(first, last, re), end;
        std::for_each(begin, end, callback);

        s.append(endOfLastMatch, last);

        return s;
    }

    template<class Traits, class CharT, class UnaryFunction>
    std::string regex_replace_f(const std::string& s, const std::basic_regex<CharT, Traits>& re, UnaryFunction f)
    {
        return regex_replace_f(s.cbegin(), s.cend(), re, f);
    }
}
