#define GLEW_STATIC

#include "main.hpp"


int __cdecl main(const int argc, const char** const argv)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, VERSION_MAJ);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, VERSION_MIN);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

    GLFWwindow* const window = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT, "Golf Game", nullptr, nullptr);
    int exit_code = 0;

    if (!window)
    {
        std::cout << "Failed to create GLFW window." << std::endl;

        exit_code = -1;
    }
    else if (!(exit_code = window_load(window)))
        while (!glfwWindowShouldClose(window))
        {
            window_process_input(window);
            window_render(window);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

    glfwTerminate();

    return exit_code;
}

int window_load(GLFWwindow* const window)
{
    glfwMakeContextCurrent(window);
    glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    });


    return 0;
}

void window_render(GLFWwindow* const)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void window_process_input(GLFWwindow* const window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
