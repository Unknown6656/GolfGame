#define GLEW_STATIC

#include "main.hpp"

unsigned int program_handle;
unsigned int VBO, VAO, EBO;


int __cdecl main(const int argc, const char** const argv)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, VERSION_MAJ);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, VERSION_MIN);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif

    GLFWwindow* const window = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT, "Golf Game", nullptr, nullptr);
    int exit_code = -1;

    if (!window)
        std::cout << "Failed to create GLFW window." << std::endl;
    else
        glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        std::cout << "Failed to initialize GLAD" << std::endl;
    else if (window)
        exit_code = window_load(window);

    if (!exit_code)
    {
        while (!glfwWindowShouldClose(window))
        {
            window_process_input(window);
            window_render(window);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        window_unload(window);
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
    glObjectLabel(GL_SHADER, shader, path.length(), path.c_str());

    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, log);

        std::cout << "failed to compile shader:" << std::endl << log << std::endl;

        return 0;
    }
    else
        return shader;
}

int get_attribute_location(const std::string name)
{
    return glGetAttribLocation(program_handle, name.c_str());
}

int window_load(GLFWwindow* const window)
{
    glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    });

    program_handle = glCreateProgram();

    const unsigned int vert_shader = compile_shader("shader.vert", GL_VERTEX_SHADER);
    const unsigned int frag_shader = compile_shader("shader.frag", GL_FRAGMENT_SHADER);

    if (!vert_shader && !frag_shader)
        return -1;

    glAttachShader(program_handle, vert_shader);
    glAttachShader(program_handle, frag_shader);
    glLinkProgram(program_handle);
    glUseProgram(program_handle);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);















    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    const int vertex_attrib = get_attribute_location("aPosition");

    glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(vertex_attrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);



    return 0;
}

void window_unload(GLFWwindow* const)
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(program_handle);
}

void window_render(GLFWwindow* const)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program_handle);
    glBindVertexArray(VAO);
    // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void window_process_input(GLFWwindow* const window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
