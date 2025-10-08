#include <glad/glad.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/io.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/ext.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <filesystem>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "GlfwWindowUtils.h"
#include "textRenderer.h"
#include "utils.h"

#define WINDOW_WIDTH 1920.0
#define WINDOW_HEIGTH 1080.0

TextRenderer textRenderer;
std::map<GLchar, Character> Characters;
GLuint textVAO, textVBO;

float mapValue(float x, float inMin, float inMax, float outMin, float outMax) {
    float t = (x - inMin) / (inMax - inMin);           // normalize to [0, 1]
    return glm::mix(outMin, outMax, t);                // map to [outMin, outMax]
}

int main(int argc, char const *argv[])
{
    glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow *window = glfwCreateWindow(1920, 1080, "Triangle Points", NULL, NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
    
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    glViewport(0, 0, 1920, 1080);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_MULTISAMPLE);
    
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
            gl_PointSize = 15.0f; // Size in pixels
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.5, 0.2, 1.0); // orange
        }
    )";
    
        // 1. Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // 2. Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // 3. Link shaders into program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

	// find path to font
    std::string font_name = std::string(RESOURCE_PATH) + "/CuteFont-Regular.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }
	
	// load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
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
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // 4. Set up vertex data
    // For points
    glm::vec3 top = glm::vec3(0.0f, 0.5f, 0.0f);
    glm::vec3 bottomLeft = glm::vec3(-0.5f, -0.5f, 0.0f);
    glm::vec3 bottomRight = glm::vec3(0.5f, -0.5f, 0.0f);


    // Text of Points
    glm::vec3 topTextCoords = glm::vec3(0.0f, 0.0f, 0.0f);
    std::string topText, bottomLeftText, bottomRightText;
    unsigned int precisionVal = 1;

    topTextCoords.x = mapValue(-0.075f, -1.0f, 1.0f, 0.0f, 1920.0f);
    topTextCoords.y = mapValue(0.59f, -1.0f, 1.0f, 0.0f, 1080.0f);
    topText = glmToText(top, precisionVal);

    glm::vec3 bottomLeftTextCoords = glm::vec3(0.0f, 0.0f, 0.0f);
    bottomLeftTextCoords.x = mapValue(-0.6, -1.0f, 1.0f, 0.0f, 1920.0f);
    bottomLeftTextCoords.y = mapValue(-0.59f, -1.0f, 1.0f, 0.0f, 1080.0f);
    bottomLeftText = glmToText(bottomLeft, precisionVal);

    glm::vec3 bottomRightTextCoords = glm::vec3(0.0f, 0.0f, 0.0f);
    bottomRightTextCoords.x = mapValue(0.415f, -1.0f, 1.0f, 0.0f, 1920.0f);
    bottomRightTextCoords.y = mapValue(-0.59f, -1.0f, 1.0f, 0.0f, 1080.0f);
    bottomRightText = glmToText(bottomRight, precisionVal);

    float startTime = glfwGetTime();
    unsigned int pointCounts = 0;
	while(!glfwWindowShouldClose(window)){

    glClearColor(0.10, 0.10, 0.10, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    

    float currentTime = glfwGetTime();
    float totalAnimDuration = 10.0f;
    float elapsed = currentTime - startTime;
    float segmentDuration = 3.0f;

    std::vector<glm::vec3> drawPoints;

    std::string text = "";
    std::string topTextString = "";
    std::string bottomRightTextString = "";

    if (elapsed < segmentDuration) {
        drawPoints.push_back(bottomLeft);
        pointCounts = 1;
    } else if (elapsed < 1.5 * segmentDuration) {
        drawPoints.push_back(bottomLeft);
        text = "(" + std::to_string(bottomLeft.x) + " " + std::to_string(bottomLeft.y) + ")";
        text = std::to_string(bottomLeft.x).substr(0, std::to_string(bottomLeft.x).find(".") + precisionVal + 1);
        textRenderer.renderText(Characters, textVAO, textVBO, bottomLeftText, bottomLeftTextCoords.x, bottomLeftTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    } 
    else if (elapsed < 2 * segmentDuration) {
        drawPoints.push_back(bottomLeft);
        drawPoints.push_back(top);
        pointCounts = 2;
        //text = glm::to_string(bottomLeft.);
        topTextString = glm::to_string(top);
        textRenderer.renderText(Characters, textVAO, textVBO, bottomLeftText, bottomLeftTextCoords.x, bottomLeftTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        textRenderer.renderText(Characters, textVAO, textVBO, topText, topTextCoords.x, topTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    } else if (elapsed < 3 * segmentDuration) {
        drawPoints.push_back(bottomLeft);
        drawPoints.push_back(top);
        drawPoints.push_back(bottomRight);
        pointCounts = 3;

        text = glm::to_string(bottomLeft);
        topTextString = glm::to_string(top);
        bottomRightTextString = glm::to_string(bottomRight);
        textRenderer.renderText(Characters, textVAO, textVBO, bottomLeftText, bottomLeftTextCoords.x, bottomLeftTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        textRenderer.renderText(Characters, textVAO, textVBO, topText, topTextCoords.x, topTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        textRenderer.renderText(Characters, textVAO, textVBO, bottomRightText, bottomRightTextCoords.x, bottomRightTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    } else {
        drawPoints.push_back(bottomLeft);
        drawPoints.push_back(top);
        drawPoints.push_back(bottomRight);
        pointCounts = 3;

        text = glm::to_string(bottomLeft);
        topTextString = glm::to_string(top);
        bottomRightTextString = glm::to_string(bottomRight);
        textRenderer.renderText(Characters, textVAO, textVBO, bottomLeftText, bottomLeftTextCoords.x, bottomLeftTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        textRenderer.renderText(Characters, textVAO, textVBO, topText, topTextCoords.x, topTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        textRenderer.renderText(Characters, textVAO, textVBO, bottomRightText, bottomRightTextCoords.x, bottomRightTextCoords.y, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    }

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Wrong one
    // glBufferData(GL_ARRAY_BUFFER, sizeof(drawPoints), &drawPoints[0], GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, drawPoints.size() * sizeof(glm::vec3), drawPoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 5. Draw
    glEnable(GL_PROGRAM_POINT_SIZE);  // This is important!
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, pointCounts);

    glfwSwapBuffers(window);
    glfwPollEvents();
    }

    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(shaderProgram); 
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glfwTerminate();
    return 0;
}

