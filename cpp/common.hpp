#pragma once

#define _USE_MATH_DEFINES
#define GLM_SWIZZLE
#define GLEW_STATIC

#define VERSION_MAJ 4
#define VERSION_MIN 6

#define F_NEAR .01f
#define F_FAR 1000.f


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <regex>
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
