#define GLEW_STATIC

#include "main.hpp"


Shader* shader_main = nullptr;
Shader* shader_post = nullptr;
unsigned int FBO, RBO, TEX; // framebuffer, renderbuffer, and texturebuffer
unsigned int VBO_golf, VAO_golf, EBO_golf; // actual geometry
unsigned int VBO_quad, VAO_quad; // post-processing quad
int seed = 420;

bool ortho = false;
float pov = 90.0f;
float rotation_angle = 10.f;
glm::vec3 look_at = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 camera_position = glm::vec3(0.f, 1.6f, 2.f);

glm::vec4 color_outside_bounds = from_argb(0xFF387B43);
glm::vec4 color_rough = from_argb(0xFF5C8C46);
glm::vec4 color_fairway = from_argb(0xFF389A3B);
glm::vec4 color_tee_box = from_argb(0xFFA8952A);
glm::vec4 color_bunker = from_argb(0xFFE6C510);
glm::vec4 color_putting_green = from_argb(0xFF36C12E);
glm::vec4 color_water = from_argb(0xFF36C9F7);

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
        glEnable(GL_DEPTH_TEST);
        glad_glDebugMessageCallback(gl_debug, nullptr);
        glfwSetErrorCallback(gl_error);

        std::cout << glGetString(GL_RENDERER) << std::endl
                  << glGetString(GL_VERSION) << std::endl
                  << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
                  << glGetString(GL_VENDOR) << std::endl << std::endl;

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
    }

    window_unload(window);
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
    course = new GolfCourse(Par::Par4, 2.5f, seed);
    course->rasterize(20, &rasterization_data);
}

int window_load(GLFWwindow* const window)
{
    glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
    glfwSetFramebufferSizeCallback(window, window_resize);

    shader_main = new Shader("shaders/shader.vert", /*"shaders/shader.geom"*/ "", "shaders/shader.frag");
    shader_post = new Shader("shaders/post-process.vert", "", "shaders/post-process.frag");

    if (!shader_main->success || !shader_post->success)
        return -1;

    shader_main->use();

    game_load();

    shader_main->set_vec4("u_colors.outside_bounds", color_outside_bounds);
    shader_main->set_vec4("u_colors.tee_box", color_tee_box);
    shader_main->set_vec4("u_colors.rough", color_rough);
    shader_main->set_vec4("u_colors.fairway", color_fairway);
    shader_main->set_vec4("u_colors.bunker", color_bunker);
    shader_main->set_vec4("u_colors.putting_green", color_putting_green);
    shader_main->set_vec4("u_colors.water", color_water);
    shader_main->set_vec2("u_dimensions", rasterization_data.dimensions);
    shader_main->set_int( "u_golf_course.par", (int)rasterization_data.par);
    shader_main->set_vec2("u_golf_course.start_position", rasterization_data.start.position);
    shader_main->set_float("u_golf_course.start_size", rasterization_data.start.size);
    shader_main->set_vec2("u_golf_course.mid1_position", rasterization_data.mid[0]);
    shader_main->set_vec2("u_golf_course.mid2_position", rasterization_data.mid[1]);
    shader_main->set_vec2("u_golf_course.end_position", rasterization_data.end.position);
    shader_main->set_float("u_golf_course.end_size", rasterization_data.end.size);


    glGenVertexArrays(1, &VAO_golf);
    glGenBuffers(1, &VBO_golf);
    glGenBuffers(1, &EBO_golf);
    glBindVertexArray(VAO_golf);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_golf);
    glBufferData(GL_ARRAY_BUFFER, rasterization_data.vertices.size() * sizeof(VertexData), &rasterization_data.vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_golf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rasterization_data.indices.size() * sizeof(int), &rasterization_data.indices[0], GL_STATIC_DRAW);

    const int vertex_position = shader_main->get_attrib(nameof(vertex_position));
    const int vertex_coords = shader_main->get_attrib(nameof(vertex_coords));
    const int vertex_color = shader_main->get_attrib(nameof(vertex_color));

    glVertexAttribPointer(vertex_position, sizeof(glm::vec3) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
    glEnableVertexAttribArray(vertex_position);
    glVertexAttribPointer(vertex_coords, sizeof(glm::vec2) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(vertex_coords);
    glVertexAttribPointer(vertex_color, sizeof(glm::vec4) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
    glEnableVertexAttribArray(vertex_color);


    shader_post->use();
    shader_post->set_int("u_screen_texture", 0);

    const int screen_position = shader_post->get_attrib(nameof(screen_position));
    const int screen_coords = shader_post->get_attrib(nameof(screen_coords));
    const glm::vec2 quad_vertices[] =
    {
        // positions             // texcoords
        glm::vec2(-1.f,  1.f),   glm::vec2(0.f, 1.f),
        glm::vec2(-1.f, -1.f),   glm::vec2(0.f, 0.f),
        glm::vec2( 1.f, -1.f),   glm::vec2(1.f, 0.f),

        glm::vec2(-1.f,  1.f),   glm::vec2(0.f, 1.f),
        glm::vec2( 1.f, -1.f),   glm::vec2(1.f, 0.f),
        glm::vec2( 1.f,  1.f),   glm::vec2(1.f, 1.f),
    };

    glGenVertexArrays(1, &VAO_quad);
    glGenBuffers(1, &VBO_quad);
    glBindVertexArray(VAO_quad);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(screen_position);
    glVertexAttribPointer(screen_position, sizeof(glm::vec2) / sizeof(float), GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(screen_coords);
    glVertexAttribPointer(screen_coords, sizeof(glm::vec2) / sizeof(float), GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (void*)sizeof(glm::vec2));


    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &TEX);
    glBindTexture(GL_TEXTURE_2D, TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, INIT_WIDTH, INIT_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TEX, 0);

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, INIT_WIDTH, INIT_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

void window_resize(GLFWwindow* const, int width, int height)
{
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_2D, TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TEX, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}

void window_unload(GLFWwindow* const)
{
    glDeleteVertexArrays(1, &VAO_golf);
    glDeleteBuffers(1, &VBO_golf);
    glDeleteBuffers(1, &EBO_golf);
    glDeleteProgram(shader_main->program_handle);
    glDeleteProgram(shader_post->program_handle);
    glDeleteFramebuffers(1, &FBO);

    delete course;
    delete shader_main;
    delete shader_post;

    course = nullptr;
    shader_main = nullptr;
    shader_post = nullptr;
}

void window_render(GLFWwindow* const window)
{
    const float time = glfwGetTime();
    int width, height;

    glfwGetWindowSize(window, &width, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glEnable(GL_DEPTH_TEST);
    glClearColor(.2f, .3f, .3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 model = glm::translate(
        glm::rotate(
            glm::mat4(1.f),
            glm::radians(rotation_angle),
            glm::vec3(0.f, 1.f, 0.f)
        ),
        glm::vec3(-.5f * rasterization_data.dimensions.x, 0.f, -.5f * rasterization_data.dimensions.y)
    );
    const glm::mat4 view = glm::lookAt(camera_position, look_at, glm::vec3(0.f, 1.f, 0.f));
    const glm::mat4 proj = ortho ? glm::ortho<float>(0, width, 0, height, F_NEAR, F_FAR)
                                 : glm::perspective(glm::radians(pov / 2), (float)width / (float)height, F_NEAR, F_FAR);

    shader_main->use();
    shader_main->set_float("u_time", time);
    shader_main->set_mat4("u_model", model);
    shader_main->set_mat4("u_view", view);
    shader_main->set_mat4("u_projection", proj);

    glBindVertexArray(VAO_golf);
    glDrawElements(GL_TRIANGLES, rasterization_data.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_post->use();

    glBindVertexArray(VAO_quad);
    glBindTexture(GL_TEXTURE_2D, TEX);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
    if (pressed(GLFW_KEY_R))
    {
        Sleep(100);

        seed += 7;

        window_unload(window);
        window_load(window);
        glfwPollEvents();
    }

#undef pressed
}
