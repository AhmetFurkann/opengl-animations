#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <glad/glad.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <string>
#include <map>

// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

class TextRenderer{

public:
    void renderText(std::map<GLchar, Character> &Characters, GLuint &VAO, GLuint &VBO,
                    std::string text, float x, float y, float scale, glm::vec3 color);
};

#endif