#define GLEW_STATIC

#include "main.hpp"

Shader* shader = nullptr;
unsigned int VBO, VAO, EBO;

bool ortho = false;
float pov = 35.0f;
float rotation_angle = 20.f;
glm::vec3 look_at = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 camera_position = glm::vec3(0.f, 2.f, 3.f);


RasterizationData rasterization_data;
GolfCourse* course = nullptr;


int __cdecl main(const int argc, const char** const argv)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, VERSION_MAJ);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, VERSION_MIN);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#ifdef DOUBLE_BUFFERING
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
#else
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
#endif
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif
    GLFWwindow* const window = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT, "Golf Game", nullptr, nullptr);
    int exit_code = -1;

    if (!window)
        std::cout << "Failed to create GLFW window." << std::endl;
    else
    {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        std::cout << "Failed to initialize GLAD" << std::endl;
    else if (window)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glad_glDebugMessageCallback(gl_debug, nullptr);
        glfwSetErrorCallback(gl_error);

        exit_code = window_load(window);
    }

    if (!exit_code)
    {
        double curr_time, disp_time = glfwGetTime();
        int frames = 0;

        while (!glfwWindowShouldClose(window))
        {
            curr_time = glfwGetTime();

            if ((curr_time - disp_time) >= FPS_INTERVAL)
            {
                std::string title = "Golf Game   [" + std::to_string(frames / (curr_time - disp_time)) + " FPS]";

                glfwSetWindowTitle(window, title.c_str());

                frames = 0;
                disp_time = curr_time;
            }
            else
                ++frames;

            window_process_input(window);
            window_render(window);
#ifdef DOUBLE_BUFFERING
            glfwSwapBuffers(window);
#else
            glFlush();
#endif
            glfwPollEvents();
        }

        window_unload(window);
    }

    delete course;
    delete shader;

    glfwTerminate();

    return exit_code;
}

void __stdcall gl_debug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*)
{
    std::string s_source = "DontCare", s_type = "DontCare", s_severity = "DontCare";

    switch (source)
    {
        case 33350: s_source = "API"; break;
        case 33351: s_source = "WindowSystem"; break;
        case 33352: s_source = "ShaderCompiler"; break;
        case 33353: s_source = "3rdParty"; break;
        case 33354: s_source = "Application"; break;
        case 33355: s_source = "Other"; break;
    }

    switch (type)
    {
        case 33356: s_type = "Error"; break;
        case 33357: s_type = "DeprecatedBehavior"; break;
        case 33358: s_type = "UndefinedBehavior"; break;
        case 33359: s_type = "Portability"; break;
        case 33360: s_type = "Performance"; break;
        case 33361: s_type = "Other"; break;
        case 33384: s_type = "Marker"; break;
        case 33385: s_type = "PushGroup"; break;
        case 33386: s_type = "PopGroup"; break;
    }

    switch (severity)
    {
        case 33387: s_severity = "Notification"; break;
        case 37190: s_severity = "High"; break;
        case 37191: s_severity = "Medium"; break;
        case 37192: s_severity = "Low"; break;
    }

    std::cout << "[GL Debug]   " << id << ": " << s_source << ", " << s_type << ", " << s_severity << " | " << message << std::endl;
}

void gl_error(int error_code, const char* message)
{
    std::cout << "[GLFW Error] " << error_code << " | " << message << std::endl;
}

void game_load()
{
    course = new GolfCourse(Par::Par4, 420);
    course->rasterize(10, &rasterization_data);
}

int window_load(GLFWwindow* const window)
{
    glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    });

    shader = new Shader("shaders/shader.vert", /*"shaders/shader.geom"*/ "", "shaders/shader.frag");

    if (shader->success)
        shader->use();
    else
        return -1;

    game_load();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, rasterization_data.vertices.size() * sizeof(VertexData), &rasterization_data.vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rasterization_data.indices.size() * sizeof(int), &rasterization_data.indices[0], GL_STATIC_DRAW);

    const int vertex_position = shader->get_attrib(nameof(vertex_position));
    const int vertex_color = shader->get_attrib(nameof(vertex_color));

    glVertexAttribPointer(vertex_position, sizeof(glm::vec3) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
    glEnableVertexAttribArray(vertex_position);

    glVertexAttribPointer(vertex_color, sizeof(glm::vec4) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(vertex_color);

    return 0;
}

void window_unload(GLFWwindow* const)
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader->program_handle);
}

void window_render(GLFWwindow* const window)
{
    const float time = glfwGetTime();
    int width, height;

    glfwGetWindowSize(window, &width, &height);

    glClearColor(.2f, .3f, .3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    const glm::mat4 model = glm::translate(
        glm::rotate(
            glm::mat4(1.f),
            glm::radians(rotation_angle),
            glm::vec3(0.f, 1.f, 0.f)
        ),
        glm::vec3(-.5f, 0.f, -.5f)
    );
    const glm::mat4 view = glm::lookAt(camera_position, look_at, glm::vec3(0.f, 1.f, 0.f));
    const glm::mat4 proj = ortho ? glm::ortho<float>(0, width, 0, height, F_NEAR, F_FAR)
                                 : glm::perspective(glm::radians(pov / 2), (float)width / (float)height, F_NEAR, F_FAR);

    shader->use();
    shader->set_float("uniform_time", time);
    shader->set_mat4("uniform_model", model);
    shader->set_mat4("uniform_view", view);
    shader->set_mat4("uniform_projection", proj);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, rasterization_data.indices.size(), GL_UNSIGNED_INT, 0);
}

void window_process_input(GLFWwindow* const window)
{
#define pressed(x) glfwGetKey(window, x) == GLFW_PRESS

    if (pressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);
    if (pressed(GLFW_KEY_1))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glLineWidth(1);
        glPointSize(1);
    }
    if (pressed(GLFW_KEY_2))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.5f);
        glPointSize(1);
    }
    if (pressed(GLFW_KEY_3))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glLineWidth(1);
        glPointSize(3.5f);
    }
    if (pressed(GLFW_KEY_4))
        ortho = false;
    if (pressed(GLFW_KEY_5))
        ortho = true;

#undef pressed
}
