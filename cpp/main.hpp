#pragma once

#define GLEW_STATIC
#define VERSION_MAJ 4
#define VERSION_MIN 6
#define INIT_WIDTH 1440
#define INIT_HEIGHT 900
#define FPS_INTERVAL .07

#define DOUBLE_BUFFERING


#include <iostream>
#include <fstream>
#include <cstdio>
#include <windows.h>
#include <string>
#include <vector>

// #include <GL/glew.h>
// #include <GL/wglew.h>
// #include <GL/glu.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <KHR/khrplatform.h>

#include "game.hpp"


int __cdecl main(const int, const char** const);

void gl_error(int, const char*);
void __stdcall gl_debug(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void*);

void game_load();

int window_load(GLFWwindow* const);
void window_unload(GLFWwindow* const);
void window_render(GLFWwindow* const);
void window_process_input(GLFWwindow* const);
