#pragma once

#define GLEW_STATIC
#define VERSION_MAJ 4
#define VERSION_MIN 6
#define INIT_WIDTH 1440
#define INIT_HEIGHT 900

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "game.hpp"


int __cdecl main(const int, const char** const);

int window_load(GLFWwindow* const);
void window_render(GLFWwindow* const);
void window_process_input(GLFWwindow* const);
