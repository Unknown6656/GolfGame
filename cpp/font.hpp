#pragma once

#include "shader.hpp"

#define ft_fail(x) std::cout << "[FreeType] " << x << std::endl


struct Character
{
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int AdvanceX;   // Offset to advance to next glyph
    unsigned int AdvanceY;   // Offset to advance to next glyph
};

struct Font
{
    std::string path;
    std::map<char, Character> characters;
    bool success = false;


    Font(const std::string& path)
        : path(path)
    {
        FT_Library ft;
        FT_Face face;

        if (FT_Init_FreeType(&ft))
            ft_fail("Could not init library.");
        else if (FT_New_Face(ft, path.c_str(), 0, &face))
            ft_fail("Failed to load font '" << path << "'.");
        else
        {
            FT_Set_Pixel_Sizes(face, 0, 48);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            for (int chr = 0; chr <= 255; ++chr)
            {
                if (FT_Load_Char(face, chr, FT_LOAD_RENDER))
                {
                    ft_fail("Failed to load glyph '" << chr << "'.");

                    continue;
                }

                unsigned int texture;

                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer
                );

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                const Character character =
                {
                    texture,
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    face->glyph->advance.x,
                    face->glyph->advance.y
                };
                characters.insert(std::pair<char, Character>((char)chr, character));
            }

            success = true;
            std::cout << characters.size() << " glyphs loaded from '" << path << "." << std::endl;

            FT_Done_Face(face);
            FT_Done_FreeType(ft);
        }
    }

    void CleanUp() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);

        for (const std::pair<char, Character>& pair : characters)
            glDeleteTextures(1, &pair.second.TextureID);
    }

    void RenderText(
        const Shader* shader,
        const std::string& text,
        const glm::vec2& position,
        const float scale,
        const glm::vec4& color,
        const int VAO,
        const int VBO
    ) const
    {
        shader->use();
        shader->set_vec4("text_color", color);
        shader->set_int("text", 0);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glActiveTexture(GL_TEXTURE0);

        float x = position.x;
        float y = position.y;

        for (std::string::const_iterator chr = text.begin(); chr != text.end(); ++chr)
        {
            const Character& character = characters.at(*chr);
            const float xpos = x + character.Bearing.x * scale;
            const float ypos = y - (character.Size.y - character.Bearing.y) * scale;
            const float w = character.Size.x * scale;
            const float h = character.Size.y * scale;

            const float vertices[6][4] =
            {
                { xpos,     ypos + h, 0.f, 0.f },
                { xpos,     ypos,     0.f, 1.f },
                { xpos + w, ypos,     1.f, 1.f },

                { xpos,     ypos + h, 0.f, 0.f },
                { xpos + w, ypos,     1.f, 1.f },
                { xpos + w, ypos + h, 1.f, 0.f }
            };

            glBindTexture(GL_TEXTURE_2D, character.TextureID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 6, vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (character.AdvanceX >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)

            if (*chr == '\r')
                x = position.x;
            else if (*chr == '\n')
            {
                x = position.x;
                y += (character.AdvanceY >> 6) * scale;
            }
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};
