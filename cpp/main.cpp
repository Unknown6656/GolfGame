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

unsigned int compile_shader(const std::string path, const GLenum type)
{
    std::ifstream stream(path);
    std::string source(
        (std::istreambuf_iterator<char>(stream)),
        (std::istreambuf_iterator<char>())
    );
    const char* c_source = source.c_str();
    const unsigned int shader = glCreateShader(type);

    glShaderSource(shader, 1, &c_source, NULL);
    glCompileShader(shader);

    int success;
    char log[1024];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, log);

        std::cout << "failed to compile shader:" << std::endl << log << std::endl;

        return 0;
    }
    else
        return shader;
}

int window_load(GLFWwindow* const window)
{
    glfwMakeContextCurrent(window);
    glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    });

    const unsigned int program = glCreateProgram();
    const unsigned int vert_shader = compile_shader("shader.vert", GL_VERTEX_SHADER);
    const unsigned int frag_shader = compile_shader("shader.frag", GL_FRAGMENT_SHADER);

    if (!vert_shader && !frag_shader)
        return -1;

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glUseProgram(program);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);


    const float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    


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
