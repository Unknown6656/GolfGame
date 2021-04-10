#define GLEW_STATIC

#include "main.hpp"

unsigned int program_handle;
unsigned int VBO, VAO, EBO;
std::vector<glm::vec3> vertices;
std::vector<int> indices;

int uniform_time;

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

unsigned int compile_shader(const std::string path, const GLenum type)
{
    std::ifstream stream(path);
    std::string source(
        (std::istreambuf_iterator<char>(stream)),
        (std::istreambuf_iterator<char>())
    );
    const char* c_source = source.c_str();
    const unsigned int shader = glCreateShader(type);

    glShaderSource(shader, 1, &c_source, nullptr);
    glCompileShader(shader);

    int success;
    char log[1024];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    glObjectLabel(GL_SHADER, shader, path.length(), path.c_str());
    glGetShaderInfoLog(shader, 1024, nullptr, log);

    if (success)
        strncpy_s(log, "  (no error)", 12);

    std::cout << "shader compile log for '" << path << "':" << std::endl << log << std::endl;

    return success ? shader : 0;
}

int get_attribute_location(const std::string name)
{
    return glGetAttribLocation(program_handle, name.c_str());
}

void game_load()
{
    course = new GolfCourse(Par::Par4, 420);
    course->rasterize(10, &vertices, &indices);
}

int window_load(GLFWwindow* const window)
{
    glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    });

    const unsigned int vert_shader = compile_shader("shaders/shader.vert", GL_VERTEX_SHADER);
    const unsigned int geom_shader = compile_shader("shaders/shader.geom", GL_GEOMETRY_SHADER);
    const unsigned int frag_shader = compile_shader("shaders/shader.frag", GL_FRAGMENT_SHADER);

    if (!vert_shader && !frag_shader) // &&!geom_shader
        return -1;

    program_handle = glCreateProgram();

    glAttachShader(program_handle, vert_shader);
    //glAttachShader(program_handle, geom_shader);
    glAttachShader(program_handle, frag_shader);
    glLinkProgram(program_handle);

    int success = 0;
    char log[1024];

    glGetProgramiv(program_handle, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(program_handle, 1024, nullptr, log);

    if (success)
        strncpy_s(log, "  (no error)", 12);

    std::cout << "program link log:" << std::endl << log << std::endl << std::endl;

    if (!success)
        return -1;

    glUseProgram(program_handle);
    glDeleteShader(vert_shader);
    glDeleteShader(geom_shader);
    glDeleteShader(frag_shader);

    uniform_time = get_attribute_location("uniform_time");

    game_load();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    const int vertex_attrib = get_attribute_location("position");

    glVertexAttribPointer(vertex_attrib, sizeof(glm::vec3) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(vertex_attrib);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    //glBindVertexArray(0);



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
    float time = glfwGetTime();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program_handle);
    glUniform1f(uniform_time, time);
    glBindVertexArray(VAO);
    // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    //glDrawArrays(GL_TRIANGLES, 0, indices.size());
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void window_process_input(GLFWwindow* const window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
}
