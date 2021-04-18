#pragma once

#include "common.hpp"


class Shader
{
public:
    const unsigned int program_handle;
    const std::string vertex_path;
    const std::string geometry_path;
    const std::string fragment_path;
    int success = 0;


    Shader(const std::string& vert_path, const std::string& geom_path, const std::string& frag_path)
        : program_handle(glCreateProgram())
        , vertex_path(vert_path)
        , geometry_path(geom_path)
        , fragment_path(frag_path)
    {
        unsigned int vert(0), geom(0), frag(0);

        if (vertex_path.length())
        {
            vert = compile_shader(vert_path, GL_VERTEX_SHADER);

            glAttachShader(program_handle, vert);
        }

        if (geom_path.length())
        {
            geom = compile_shader(geom_path, GL_GEOMETRY_SHADER);

            glAttachShader(program_handle, geom);
        }

        if (frag_path.length())
        {
            frag = compile_shader(frag_path, GL_FRAGMENT_SHADER);

            glAttachShader(program_handle, frag);
        }

        glLinkProgram(program_handle);

        char log[1024];

        glGetProgramiv(program_handle, GL_LINK_STATUS, &success);
        glGetProgramInfoLog(program_handle, 1024, nullptr, log);

        if (success)
            strncpy_s(log, "  (no error)", 12);

        std::cout << "program link log:" << std::endl << log << std::endl << std::endl;

        glUseProgram(program_handle);
        glDeleteShader(vert);
        glDeleteShader(geom);
        glDeleteShader(frag);
    }

    inline void use() const noexcept
    {
        glUseProgram(program_handle);
    }

    inline int get_attrib(const std::string& name) const noexcept
    {
        return glGetAttribLocation(program_handle, name.c_str());
    }

    template <typename WholeVertex, typename Element>
    void set_attrib_f(const std::string& name, Element WholeVertex::* const member_ptr) const noexcept
    {
        const int attrib = get_attrib(name);
        const void* offset = &(((WholeVertex*)nullptr)->*member_ptr);

        glVertexAttribPointer(
            attrib,
            sizeof(Element) / sizeof(float),
            GL_FLOAT,
            GL_FALSE,
            sizeof(WholeVertex),
            offset
        );
        glEnableVertexAttribArray(attrib);
    }

    template <typename WholeVertex, typename Element>
    void set_attrib_i(const std::string& name, Element WholeVertex::* const member_ptr) const noexcept
    {
        const int attrib = get_attrib(name);
        const void* offset = &(((WholeVertex*)nullptr)->*member_ptr);

        glVertexAttribIPointer(
            attrib,
            sizeof(Element) / sizeof(int),
            GL_INT,
            sizeof(WholeVertex),
            offset
        );
        glEnableVertexAttribArray(attrib);
    }

    inline int get_uniform(const std::string& name) const noexcept
    {
        return glGetUniformLocation(program_handle, name.c_str());
    }

    void set_bool(const std::string& name, const bool value) const noexcept
    {
        set_int(name, (int)value);
    }

    void set_int(const std::string& name, const int value) const noexcept
    {
        glUniform1i(get_uniform(name), value);
    }

    void set_float(const std::string& name, const float value) const noexcept
    {
        glUniform1f(get_uniform(name), value);
    }

    void set_vec2(const std::string& name, const glm::vec2& value) const noexcept
    {
        glUniform2f(get_uniform(name), value.x, value.y);
    }

    void set_vec3(const std::string& name, const glm::vec3& value) const noexcept
    {
        glUniform3f(get_uniform(name), value.x, value.y, value.z);
    }

    void set_vec4(const std::string& name, const glm::vec4& value) const noexcept
    {
        glUniform4f(get_uniform(name), value.x, value.y, value.z, value.w);
    }

    void set_mat2(const std::string& name, const glm::mat2& value) const noexcept
    {
        glUniformMatrix2fv(get_uniform(name), 1, false, (float*)&value);
    }

    void set_mat3(const std::string& name, const glm::mat3& value) const noexcept
    {
        glUniformMatrix3fv(get_uniform(name), 1, false, (float*)&value);
    }

    void set_mat4(const std::string& name, const glm::mat4& value) const noexcept
    {
        glUniformMatrix4fv(get_uniform(name), 1, false, (float*)&value);
    }

private:
    static std::string read_file(const std::string& path)
    {
        std::ifstream stream(path);

        return std::string(
            (std::istreambuf_iterator<char>(stream)),
            (std::istreambuf_iterator<char>())
        );
    }

    static unsigned int compile_shader(const std::string& path, const GLenum type)
    {
        using namespace std::regex_constants;
        using namespace std;

        const regex reg_include(R"R((^|\r?\n)(?:\s*#include\s*"(.+)"\s*)(?=\r?\n|$))R", icase);
        string source = read_file(path);
        bool changed;

        do
        {
            changed = false;
            source = regex_replace_f(source, reg_include, [&](const smatch& m) -> string
            {
                const string p = path + "/../" + m[2].str();
                string content = read_file(p);

                trim(content);

                changed = true;

                return "\n//---- begin include '" + p + "' ----\n" + content + "\n//---- end include '" + p + "' ----\n";
            });
        }
        while (changed);

        // source = regex_replace_f(source, regex(R"((\s*\/\/.*(\r?\n|$)|[\r\n]+))"), [](const smatch&) { return "\n"; });
        // trim(source);

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

        cout << "shader compile log for '" << path << "':" << endl << log << endl;

        return success ? shader : 0;
    }
};

