#pragma once

#include "common.hpp"

#define ft_fail(x) std::cout << "[FreeType] " << x << std::endl


struct Character
{
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

struct Font
{
    std::string path;
    std::map<char, Character> characters;
    FT_Library ft;
    FT_Face face;
    bool success = false;


    Font(const std::string& path)
        : path(path)
    {
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
                    face->glyph->advance.x
                };
                characters.insert(std::pair<char, Character>((char)chr, character));
            }

            success = true;
            std::cout << characters.size() << " glyphs loaded from '" << path << "." << std::endl;
        }
    }

    void CleanUp() const
    {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        glBindTexture(GL_TEXTURE_2D, 0);

        for (const std::pair<char, Character>& pair : characters)
            glDeleteTextures(1, &pair.second.TextureID);
    }
};
