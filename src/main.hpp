#pragma once

#define INIT_WIDTH 1920
#define INIT_HEIGHT 1080
#define FPS_DISPLAY_INTERVAL .07
#define FPS_TARGET 360.0
#define TPS_TARGET 60.0

#define CAMERA_SPEED .05
#define ROTATION_SPEED .02
#define STRENGTH_SPEED .01
#define BALL_ANIM_DURATION 1100
#define WALK_ANIM_DURATION 2000
#define PLAYER_SIZE .13f

//#define MULTI_SAMPLE 4
#define DOUBLE_BUFFERING


#include "game.hpp"
#include "font.hpp"
#include "shader.hpp"


int __cdecl main(const int, const char** const);

void gl_error(int, const char*);
void __stdcall gl_debug(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void*);

int window_load(GLFWwindow* const);
void window_unload(GLFWwindow* const);
void window_render(GLFWwindow* const, const float);
void window_resize(GLFWwindow* const, int, int);
void window_process_input(GLFWwindow* const, const float);

void reset_player();

