#define GLEW_STATIC

#include "main.hpp"



Shader* shader_main = nullptr;
Shader* shader_post = nullptr;
Shader* shader_font = nullptr;
ImageTexture* img_clubs = nullptr;
ImageTexture* img_flagpole = nullptr;
unsigned int FBO, RBO, TEX; // framebuffer, renderbuffer, and texturebuffer
unsigned int VBO_golf, VAO_golf, EBO_golf; // golf course geometry
unsigned int VBO_quad, VAO_quad; // post-processing quad
unsigned int TEX_surface;
int seed = randf() * 420 + 315;

bool effects = true;
float fov = 90.0f;
float rotation_angle = 10.f;
glm::vec3 light_position = glm::vec3(0.f, 3.f, -2.f);
glm::vec3 camera_position = glm::vec3(0.f, 1.6f, 2.3f);
glm::mat4 parabola_transform = glm::mat4(1.f);

glm::vec4 color_outside_bounds = from_argb(0xFF043925);
glm::vec4 color_rough = from_argb(0xFF025A34);
glm::vec4 color_fairway = from_argb(0xFF227B43);
glm::vec4 color_tee_box = from_argb(0xFF6BA453);
glm::vec4 color_bunker = from_argb(0xFFDDC69A);
glm::vec4 color_putting_green = from_argb(0xFF8EC375);
glm::vec4 color_water = from_argb(0xFF36C9F7);
glm::vec4 color_sun = from_argb(0xFFF7DB09);

RasterizationData rasterization_data;
GolfCourse* course = nullptr;
glm::vec2 player_ball_target;
glm::vec2 player_position;
GolfClubType player_club;
float player_orientation;
float player_strength;
float ball_position;
volatile bool is_animating;

Font* font_main = nullptr;

//constexpr const char font_path[] = "assets/arcade.ttf";
constexpr const char font_path[] = "assets/basis33.regular.ttf";
constexpr const char help_text[] = R"(Keyboard bindings:
[W] [A] [S] [D]   Move the camera
[E] [R]           Zoom in or out
[LEFT] [RIGHT]    Turn the player
[C] [V]           Switch golf clubs
[F] [G]           Change player/swing strength
[ENTER]           Play / execute swing
[T]               Reset player
[ESCAPE]          Quit game
[F4]              Enable/Disable visual effects
)";


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
#ifdef MULTI_SAMPLE
    glfwWindowHint(GLFW_SAMPLES, MULTI_SAMPLE);
#endif
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_TRUE);
    glfwWindowHint(GLFW_REFRESH_RATE, FPS_TARGET);
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

        std::cout << glGetString(GL_RENDERER) << std::endl
                  << glGetString(GL_VERSION) << std::endl
                  << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
                  << glGetString(GL_VENDOR) << std::endl << std::endl;

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_TEXTURE_2D);
        // glShadeModel(GL_SMOOTH);
        // glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

        font_main = new Font(font_path);
        exit_code = window_load(window);
    }

    if (!exit_code)
    {
        double curr_time = glfwGetTime();
        double inpt_time = curr_time;
        double disp_time = curr_time;
        int frames = 0;

        reset_player();

        while (!glfwWindowShouldClose(window))
        {
            if ((curr_time - disp_time) >= FPS_DISPLAY_INTERVAL)
            {
                glfwSetWindowTitle(window, format("Golf Game   [%f FPS]", frames / (curr_time - disp_time)).c_str());

                frames = 0;
                disp_time = curr_time;
            }
            else
                ++frames;

            if ((curr_time - inpt_time) * TPS_TARGET >= 1)
            {
                window_process_input(window, curr_time);

                inpt_time = curr_time;
            }

            window_render(window, curr_time);
#ifdef DOUBLE_BUFFERING
            glfwSwapBuffers(window);
#else
            glFlush();
#endif
            glfwPollEvents();

            constexpr double ifps = 1.0 / FPS_TARGET;

            while (glfwGetTime() - curr_time <= ifps)
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(ifps * 330)));

            curr_time = glfwGetTime() + glfwGetTime() - curr_time - ifps; // add overhead
        }
    }

    window_unload(window);
    glfwTerminate();
    font_main->CleanUp();

    delete font_main;

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

void update_parabola()
{
    // https://danbubanygolf.com/club-fitting-variables-no-5-6-7/
    const float angle_of_attack = glm::radians(player_club == GolfClubType::Wood1 ? 10.f
                                             : player_club == GolfClubType::Wood2 ? 18.f
                                             : player_club == GolfClubType::Iron3 ? 21.f
                                             : player_club == GolfClubType::Iron4 ? 24.f
                                             : player_club == GolfClubType::Iron5 ? 27.f
                                             : player_club == GolfClubType::Iron6 ? 30.f
                                             : player_club == GolfClubType::Iron7 ? 34.f
                                             : player_club == GolfClubType::Iron8 ? 38.f
                                             : player_club == GolfClubType::Iron9 ? 42.f
                                             : player_club == GolfClubType::PitchingWedge ? 46.f
                                             : player_club == GolfClubType::SandWedge ? 54.f
                                             : player_club == GolfClubType::Putter ? .6f
                                             : 90.f);
    float distance = player_strength * map(angle_of_attack, .943, .17, .25, 1.5);
    const float height = tan(angle_of_attack) * distance * .25f;
    const float sinφ = sin(player_orientation);
    const float cosφ = cos(player_orientation);

    if (player_club == GolfClubType::Putter)
        distance = player_strength * .12f;

    parabola_transform = glm::mat4(
                                  distance * cosφ,    0.f,         distance * sinφ, 0.f,
                                              0.f, height,                     0.f, 0.f,
                                            -sinφ,    0.f,                    cosφ, 0.f,
        player_position.x - course->_length * .5f,    0.f, player_position.y - .5f, 1.f
    );

    const float target_distance = distance + randf() * .1f;
    const float target_angle = player_orientation + randf() * .1f;

    // player_ball_target = player_position + glm::vec2(cos(target_angle), sin(target_angle)) * target_distance;
    player_ball_target = player_position + glm::vec2(cos(player_orientation), sin(player_orientation)) * distance;

    shader_main->use();
    shader_main->set_float("u_parabola_height", height);
}

void animate_ball()
{
    if (!is_animating)
    {
        is_animating = true;

        std::thread([&]()
            {
                constexpr int TOTAL_MS = 700;
                const float t_start = glfwGetTime();

                ball_position = 0;

                while (ball_position < 1)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));

                    ball_position = (glfwGetTime() - t_start) * 1000.f / TOTAL_MS;
                }

                ball_position = -1;
                is_animating = false;
                player_position = player_ball_target;
            }).detach();
    }
}

int window_load(GLFWwindow* const window)
{
    glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
    glfwSetFramebufferSizeCallback(window, window_resize);

    std::cout << "--------------------------------------------------------------------" << std::endl;

    shader_main = new Shader("shaders/shader.vert", /*"shaders/shader.geom"*/ "", "shaders/shader.frag");
    shader_post = new Shader("shaders/post-process.vert", "", "shaders/post-process.frag");
    shader_font = new Shader("shaders/font.vert", "", "shaders/font.frag");

    std::cout << "--------------------------------------------------------------------" << std::endl;

    if (!shader_main->success || !shader_post->success || !shader_font->success)
        return -1;
    else
        shader_main->use();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);


    /////////////////////////////////// SET UP MAIN GEOMETRY ///////////////////////////////////

    course = new GolfCourse(Par::Par4, 2.5f, seed);
    course->rasterize(20, 128, &rasterization_data);
    player_position = course->_course_start_position.position;

    const int vertex_index_offs = rasterization_data.vertices.size();
    const std::vector<VertexData> vertices =
    {
        VertexData(glm::vec3(0, 0, 0), glm::vec2(0, 0), VertexType::Parabola),
        VertexData(glm::vec3(0, 1, 0), glm::vec2(0, 1), VertexType::Parabola),
        VertexData(glm::vec3(1, 1, 0), glm::vec2(1, 1), VertexType::Parabola),
        VertexData(glm::vec3(1, 0, 0), glm::vec2(1, 0), VertexType::Parabola),
        VertexData(glm::vec3(0, 0, 0), glm::vec2(0, 1), VertexType::Player),
        VertexData(glm::vec3(0, 1, 0), glm::vec2(0, 0), VertexType::Player),
        VertexData(glm::vec3(1, 1, 0), glm::vec2(1, 0), VertexType::Player),
        VertexData(glm::vec3(1, 0, 0), glm::vec2(1, 1), VertexType::Player),
        VertexData(glm::vec3(0, 0, 0), glm::vec2(0, 1), VertexType::Flagpole),
        VertexData(glm::vec3(0, .3, 0), glm::vec2(0, 0), VertexType::Flagpole),
        VertexData(glm::vec3(.1, .3, 0), glm::vec2(1, 0), VertexType::Flagpole),
        VertexData(glm::vec3(.1, 0, 0), glm::vec2(1, 1), VertexType::Flagpole),
    };
    const std::vector<int> indices =
    {
        vertex_index_offs, vertex_index_offs + 1, vertex_index_offs + 2,
        vertex_index_offs, vertex_index_offs + 2, vertex_index_offs + 3,
        vertex_index_offs + 4, vertex_index_offs + 5, vertex_index_offs + 6,
        vertex_index_offs + 4, vertex_index_offs + 6, vertex_index_offs + 7,
        vertex_index_offs + 8, vertex_index_offs + 9, vertex_index_offs + 10,
        vertex_index_offs + 8, vertex_index_offs + 10, vertex_index_offs + 11,
    };

    rasterization_data.vertices.insert(rasterization_data.vertices.end(), vertices.begin(), vertices.end());
    rasterization_data.indices.insert(rasterization_data.indices.end(), indices.begin(), indices.end());


    /////////////////////////////////// SET UP MAIN SHADER ///////////////////////////////////

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &TEX_surface);
    glBindTexture(GL_TEXTURE_2D, TEX_surface);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glm::vec4 default_color((float)SurfaceType::OutsideCourse);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (float*)&default_color);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        rasterization_data.surface.width,
        rasterization_data.surface.height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        (void*)&rasterization_data.surface.pixels[0]
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    glGenVertexArrays(1, &VAO_golf);
    glGenBuffers(1, &VBO_golf);
    glGenBuffers(1, &EBO_golf);
    glBindVertexArray(VAO_golf);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_golf);
    glBufferData(GL_ARRAY_BUFFER, rasterization_data.vertices.size() * sizeof(VertexData), &rasterization_data.vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_golf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rasterization_data.indices.size() * sizeof(int), &rasterization_data.indices[0], GL_STATIC_DRAW);

    shader_main->set_attrib_f("vertex_position", &VertexData::position);
    shader_main->set_attrib_f("vertex_coords", &VertexData::coords);
    shader_main->set_attrib_i("vertex_type", &VertexData::type);
    shader_main->set_int("tex_surface", 0);
    shader_main->set_vec4("u_colors.outside_bounds", color_outside_bounds);
    shader_main->set_vec4("u_colors.tee_box", color_tee_box);
    shader_main->set_vec4("u_colors.rough", color_rough);
    shader_main->set_vec4("u_colors.fairway", color_fairway);
    shader_main->set_vec4("u_colors.bunker", color_bunker);
    shader_main->set_vec4("u_colors.putting_green", color_putting_green);
    shader_main->set_vec4("u_colors.water", color_water);
    shader_main->set_vec4("u_colors.sun", color_sun);
    shader_main->set_vec2("u_dimensions", rasterization_data.dimensions);
    img_flagpole = new ImageTexture("assets/flagpole.png", shader_main, "tex_flagpole", 2);

    glBindTexture(GL_TEXTURE_2D, 0);

    rasterization_data.surface.pixels.clear();
    rasterization_data.surface.pixels.~vector();


    if (!font_main->SetUp(shader_font))
        return -1;

    /////////////////////////////////// SET UP POST-PROCESSING SHADER ///////////////////////////////////
    img_clubs = new ImageTexture("assets/clubs.png", shader_post, "u_texture_clubs", 2);
    shader_post->use();
    shader_post->set_int("u_texture_screen", 0);
    shader_post->set_float("u_pixelation_factor", PIXELATION_FACTOR);

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


    /////////////////////////////////// SET UP POST-PROCESSING FRAME BUFFERS ETC. ///////////////////////////////////

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &TEX);
#ifdef MULTI_SAMPLE
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, TEX);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MULTI_SAMPLE, GL_RGB, INIT_WIDTH, INIT_HEIGHT, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, TEX, 0);
#else
    glBindTexture(GL_TEXTURE_2D, TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, INIT_WIDTH, INIT_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TEX, 0);
#endif

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
#ifdef MULTI_SAMPLE
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MULTI_SAMPLE, GL_DEPTH24_STENCIL8, INIT_WIDTH, INIT_HEIGHT);
#else
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, INIT_WIDTH, INIT_HEIGHT);
#endif
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifdef MULTI_SAMPLE
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
#else
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

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
    delete img_clubs;
    delete img_flagpole;
    delete shader_main;
    delete shader_post;
    delete shader_font;

    course = nullptr;
    img_clubs = nullptr;
    img_flagpole = nullptr;
    shader_main = nullptr;
    shader_post = nullptr;
    shader_font = nullptr;
}

inline void render_text_2D(GLFWwindow* const window, const std::string& text, const float x, const float y, const float size, const glm::vec4& color)
{
    int width, height;

    glfwGetWindowSize(window, &width, &height);
    font_main->RenderText(shader_font, nullptr, glm::vec2(width, height), text, glm::vec2(x, y), size, color);
}

inline void render_text_3D(const std::string& text, glm::mat4 camera, const float x, const float y, const float size, const glm::vec4& color)
{
    camera = glm::translate(glm::mat4(1.f), glm::vec3(x, y, 0.f)) * camera;
    camera = glm::scale(camera, glm::vec3(size));

    font_main->RenderText(shader_font, &camera, glm::vec2(0.f), text, glm::vec3(0.f), 1.f, color);
}

void window_render(GLFWwindow* const window, const float time)
{
    int width, height;

    glfwGetWindowSize(window, &width, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glEnable(GL_DEPTH_TEST);
    glClearColor(.2f, .3f, .3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::vec3 look_at = glm::vec3((parabola_transform * glm::vec4(0.f, 0.f, 0.f, 1.f)).xyz * .5f);
    const glm::mat4 model = glm::translate(
        glm::mat4(1.f),
        glm::vec3(-.5f * rasterization_data.dimensions.x, 0.f, -.5f * rasterization_data.dimensions.y)
    );
    const glm::mat4 view = glm::rotate(
        glm::lookAt(camera_position, look_at, glm::vec3(0.f, 1.f, 0.f)),
        glm::radians(rotation_angle),
        glm::vec3(0.f, 1.f, 0.f)
    );
    const glm::mat4 proj = glm::perspective(glm::radians(fov / 2), (float)width / (float)height, F_NEAR, F_FAR);

    const glm::vec2 flagpole_pos = rasterization_data.end.position - .5f * rasterization_data.dimensions;
    const float flagpole_angle = atan2f(flagpole_pos.y - camera_position.z, flagpole_pos.x - camera_position.x);
    const glm::mat4 flagpole(
        -sin(flagpole_angle), 0.f,  cos(flagpole_angle), 0.f,
                         0.f, 1.f,                  0.f, 0.f,
        -cos(flagpole_angle), 0.f, -sin(flagpole_angle), 0.f,
              flagpole_pos.x, 0.f,       flagpole_pos.y, 1.f
    );

    shader_main->use();
    shader_main->set_float("u_time", time);
    shader_main->set_int("u_effects", effects);
    shader_main->set_mat4("u_model", model);
    shader_main->set_mat4("u_parabola", parabola_transform);
    //shader_main->set_mat4("u_player", );
    shader_main->set_mat4("u_flagpole", flagpole);
    shader_main->set_mat4("u_view", view);
    shader_main->set_mat4("u_projection", proj);
    shader_main->set_vec3("u_camera_position", camera_position);
    shader_main->set_vec3("u_light_position", light_position);
    shader_main->set_float("u_ball_position", ball_position);
    img_flagpole->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TEX_surface);
    glBindVertexArray(VAO_golf);
    glDrawElements(GL_TRIANGLES, rasterization_data.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_post->use();
    shader_post->set_float("u_time", time);
    shader_post->set_int("u_width", width);
    shader_post->set_int("u_height", height);
    shader_post->set_int("u_effects", effects);
    shader_post->set_int("u_selected_club", (int)player_club);
    shader_post->set_float("u_player_strength", player_strength);
    img_clubs->bind();

    glBindVertexArray(VAO_quad);
    glActiveTexture(GL_TEXTURE0);
#ifdef MULTI_SAMPLE
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, TEX);
#else
    glBindTexture(GL_TEXTURE_2D, TEX);
#endif
    glDrawArrays(GL_TRIANGLES, 0, 6);
#ifdef MULTI_SAMPLE
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
#else
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

    constexpr const char* lol[12] = { "?", "D", "W2", "I3", "I4", "I5", "I6", "I7", "I8", "I9", "P", "W" };

    if (!is_animating)
        render_text_2D(window, help_text, 10, height - 70, 1.3, from_argb(0xffbbbbbb));
    //render_text_2D(window, format("club: %s, pos: %f|%f, or: %f, st: %f", lol[(int)player_club], player_position.x, player_position.y, player_orientation, player_strength), 10, 10, 1.5, from_argb(0xffff8888));
}

void window_process_input(GLFWwindow* const window, const float time)
{
#define pressed(x) glfwGetKey(window, x) == GLFW_PRESS

    if (pressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (is_animating)
        return;

    if (pressed(GLFW_KEY_F9))
    {
        Sleep(100);

        seed += 7;

        window_unload(window);
        window_load(window);
        glfwPollEvents();

        return;
    }

    if (pressed(GLFW_KEY_F1))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glLineWidth(1);
        glPointSize(1);
    }
    else if (pressed(GLFW_KEY_F2))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.5f);
        glPointSize(1);
    }
    else if (pressed(GLFW_KEY_F3))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glLineWidth(1);
        glPointSize(3.5f);
    }

    if (pressed(GLFW_KEY_F4))
    {
        Sleep(120);
        effects ^= true;
    }

    if (pressed(GLFW_KEY_A))
        camera_position.x -= CAMERA_SPEED;
    if (pressed(GLFW_KEY_D))
        camera_position.x += CAMERA_SPEED;
    if (pressed(GLFW_KEY_W))
    {
        camera_position.y -= CAMERA_SPEED * .5;
        camera_position.z -= CAMERA_SPEED;
    }
    if (pressed(GLFW_KEY_S))
    {
        camera_position.y += CAMERA_SPEED * .5;
        camera_position.z += CAMERA_SPEED;
    }
    if (pressed(GLFW_KEY_R))
        fov += CAMERA_SPEED * 12;
    if (pressed(GLFW_KEY_E))
        fov -= CAMERA_SPEED * 12;

    if (pressed(GLFW_KEY_T))
        reset_player();
    if (pressed(GLFW_KEY_RIGHT))
        player_orientation -= ROTATION_SPEED;
    if (pressed(GLFW_KEY_LEFT))
        player_orientation += ROTATION_SPEED;

    if (pressed(GLFW_KEY_F))
        player_strength -= STRENGTH_SPEED;
    if (pressed(GLFW_KEY_G))
        player_strength += STRENGTH_SPEED;

    if (pressed(GLFW_KEY_C) && player_club > GolfClubType::Wood1)
    {
        Sleep(120);
        --*(int*)&player_club;
    }
    if (pressed(GLFW_KEY_V) && player_club < GolfClubType::Putter)
    {
        Sleep(120);
        ++*(int*)&player_club;
    }

    if (pressed(GLFW_KEY_ENTER))
        animate_ball();

    fov = glm::clamp(fov, 60.f, 100.f);
    player_orientation = modpos(player_orientation, 2 * M_PI);
    player_strength = glm::clamp(player_strength, .3f, 1.f);
    camera_position = glm::clamp(camera_position, glm::vec3(-2.f, .6f, .3f), glm::vec3(2.f, 1.6f, 2.3f));

    update_parabola();

#undef pressed
}

void reset_player()
{
    player_position = course->_course_start_position.position;
    player_orientation = 0;
    player_club = GolfClubType::Driver;
    player_strength = 1;
    ball_position = -1;
    is_animating = false;
}
