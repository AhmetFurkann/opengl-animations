#include <glad/glad.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include "textRenderer.h"
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/io.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/ext.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <filesystem>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <algorithm>

#include <cmath>

#include "GlfwWindowUtils.h"
#include "textRenderer.h"
#include "utils.h"

#define WINDOW_WIDTH 3840.0f
#define WINDOW_HEIGHT 2160.0f

TextRenderer textRenderer;
std::map<GLchar, Character> Characters;
GLuint textVAO, textVBO;

float screenCenterX = WINDOW_WIDTH  / 2.0f;
float screenCenterY = WINDOW_HEIGHT / 2.0f;
glm::vec2 screenCenter = glm::vec2(screenCenterX, screenCenterY);

float mapValue(float x, float inMin, float inMax, float outMin, float outMax) {
    float t = (x - inMin) / (inMax - inMin);           // normalize to [0, 1]
    return glm::mix(outMin, outMax, t);                // map to [outMin, outMax]
}

float radiansToDegrees(float radians) {
    return radians * 180.0 / M_PI;  // Use M_PI from cmath for pi
}

float calculateAngle(glm::vec2 point1, glm::vec2 point2){
    float radiansValue = atan2(point1.y - point2.y, point1.x - point2.x);
    float degree = radiansToDegrees(radiansValue);
    //if (degree < 0.0f)
    //    degree += 360.0f;
    return degree;
}

struct AnimationSegment {
    float startTime;
    float endTime;
    bool isActive(float time) const {
        return time >= startTime && time <= endTime;
    }
};

class AnimationTimeline {
private:

    unsigned int numberOfLine = 0;
    float xScaleRate = 4.0f;
    float yScaleRate = 2.0f;

public:
    std::vector<AnimationSegment> segments;
    unsigned int getNumberOfLine(){
        return numberOfLine;
    }

    void addSegment(float start, float end) {
        segments.push_back({ start, end });
    }

    glm::mat4 evaluate(float elapsedTime, GLuint &uModelLoc, GLuint &quadTriVAO, std::vector<glm::vec2> &midPoints) {
        glm::mat4 model(1.0f);
        numberOfLine = 0;
        for (std::size_t i = 0; i < segments.size(); ++i) {
            float animStartTime = segments[i].startTime;
            float animStopTime = segments[i].endTime;
            glm::vec2 quadCenter = glm::vec2(midPoints[i].x, midPoints[i].y);
            glm::vec2 baseMidQuad = glm::vec2(midPoints[0].x, midPoints[0].y);

            if (segments[i].isActive(elapsedTime)) {
                if (i == 0)
                    animateFirstQuad(model, uModelLoc, quadTriVAO, quadCenter);
                else
                    animateQuad(elapsedTime, model, uModelLoc, quadTriVAO, animStartTime,
                                animStopTime, quadCenter, baseMidQuad);

                break;  // Only one segment active per time
            }
            else{
                if (i != 0)
                    numberOfLine++;
            }
        }
    }

    void animateFirstQuad(glm::mat4 &model, GLuint &uModelLoc, GLuint &quadTriVAO, glm::vec2 quadCenter){
        model = glm::translate(model, glm::vec3(screenCenter, 0.0f));
        model = glm::scale(model, glm::vec3(this->xScaleRate, this->yScaleRate, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(-quadCenter, 0.0f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(quadTriVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * 1);
    }

    void animateQuad(float elapsedTime, glm::mat4 &model, GLuint &uModelLoc, GLuint &quadTriVAO,
                     float animTimeStart, float animTimeStop, glm::vec2 quadCenter, glm::vec2 baseMidQuad){
        // Start by zooming out from 4x to 1x --> X
        // Start by zooming out from 2x to 1x --> Y
        float scaleValueX = mapValue(elapsedTime, animTimeStart, animTimeStop, xScaleRate, 1.0f);
        float scaleValueY = mapValue(elapsedTime, animTimeStart, animTimeStop, yScaleRate, 1.0f);
        float angle = calculateAngle(quadCenter, screenCenter);
        // atan2 returnss values in the range of -π to +π (i.e., [-180°, +180°])
        // baseMidQuad is 90° therefore the angle needs to be adjusted according to!
        // And the rotation val is fixed 90° because of that.
        if (angle < 0.0f)
            angle += 180.0f;
        float roatationVal = mapValue(elapsedTime, animTimeStart, animTimeStop, 90.0f, angle);

        // Find out how much we need to move on both X and Y axis
        float tx = quadCenter.x - screenCenterX;
        float ty = quadCenter.y - screenCenterY;
        float txMapped = mapValue(elapsedTime, animTimeStart, animTimeStop, 0.0f, tx);
        float tyMapped = mapValue(elapsedTime, animTimeStart, animTimeStop, 0.0f, ty);

        // The transformation in OpenGL is applied in reverse order
        // Keep in mind that!
        model = glm::translate(model, glm::vec3(txMapped, tyMapped, 0.0f));
        model = glm::translate(model, glm::vec3(screenCenter, 0.0f));
        model = glm::scale(model, glm::vec3(scaleValueX, scaleValueY, 0.0f));
        model = glm::rotate(model, glm::radians(roatationVal), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(-baseMidQuad, 0.0f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(quadTriVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * 1);
    }

    void clear() {
        segments.clear();
    }

};



int main(int argc, char const *argv[])
{
    glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "QuadCircle Demo", NULL, NULL);
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
        uniform mat4 uProjection;
        uniform mat4 uModel;
        void main() {
            gl_Position = uProjection * uModel * vec4(aPos, 1.0);
            gl_PointSize = 5.0f; // Size in pixels
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

    GLuint projLoc = glGetUniformLocation(shaderProgram, "uProjection");
    GLuint uModelLoc = glGetUniformLocation(shaderProgram, "uModel");

    GLuint quadTriVAO, quadTriVBO;

    glGenVertexArrays(1, &quadTriVAO);
    glGenBuffers(1, &quadTriVBO);

    std::vector<float> circleLineCoords;
    std::vector<glm::vec2> midPoints;

    // Creating circle part with quads
    for (float i=0.0f; i<360.0f; i++){
        Point startPoint = polartToCartesien(screenCenterX, screenCenterY, 200.0f, i);
        float angleRad = std::atan2(startPoint.y, startPoint.x);

        float thickness = 10.0f;

        Point endPoint = polartToCartesien(screenCenterX, screenCenterY, 450.0f, i);
        createLineWithQuads(glm::vec3(startPoint.x, startPoint.y, 0.0f),
                            glm::vec3(endPoint.x, endPoint.y, 0.0f),
                            thickness,
                            circleLineCoords,
                            midPoints);
    }

    glBindVertexArray(quadTriVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadTriVBO);
    glBufferData(GL_ARRAY_BUFFER, circleLineCoords.size() * sizeof(float), circleLineCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)3);
    glEnableVertexAttribArray(1);


    glm::mat4 projection = glm::ortho(
        0.0f, WINDOW_WIDTH,   // left, right
        0.0f, WINDOW_HEIGHT,  // bottom, top
        -1.0f, 1.0f           // near, far
    );

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    float startTime = glfwGetTime();

    AnimationTimeline timeline;

    float time_remaining = 2.0f;
    timeline.addSegment(0.0f, 2.0f);
    for (float i = 1; i<=360; i++){
        if (i < 30){
            time_remaining *=  0.9f;
        }
        else if (i > 330)
        {
            time_remaining *=  1.05f;
        }
        
        float previousEnd = timeline.segments[i-1].endTime;
        timeline.addSegment(previousEnd, previousEnd + time_remaining);
    }

    glm::mat4 model = glm::mat4(1.0f);

	while(!glfwWindowShouldClose(window)){

        float currentTime = glfwGetTime();
        float elapsed = currentTime - startTime;

        glClearColor(0.10, 0.10, 0.10, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        timeline.evaluate(elapsed, uModelLoc, quadTriVAO, midPoints);
        unsigned int numberOfStaticLines = timeline.getNumberOfLine();

        model = glm::mat4(1.0f);
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(quadTriVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * numberOfStaticLines);

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

