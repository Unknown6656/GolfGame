#pragma once

#define GLEW_STATIC
#define VERSION_MAJ 4
#define VERSION_MIN 6
#define INIT_WIDTH 1440
#define INIT_HEIGHT 900

#include <iostream>
#include <fstream>
#include <cstdio>
#include <windows.h>

//#include <gl.h>
#include <GL/glew.h>
#include <GL/wglew.h>
// #include <GL/glu.h>
#include <GLFW/glfw3.h>


#include "game.hpp"


int __cdecl main(const int, const char** const);

unsigned int compile_shader(const std::string, const GLenum);

int window_load(GLFWwindow* const);
void window_render(GLFWwindow* const);
void window_process_input(GLFWwindow* const);
