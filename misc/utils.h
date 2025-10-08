#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <glm/glm/glm.hpp>
#include <vector>

std::string glmToText(glm::vec3 point, uint precisionVal);

struct Point
{
  float x;
  float y;
  float z {0.0f};
};

struct Color{
    uint_fast8_t red;
    uint_fast8_t green;
    uint_fast8_t blue;
    uint_fast8_t opacity;
};

float normalize_value(float value, float r_min, float r_max, float t_min, float t_max);
float degreeToRad(float angle);
Point polartToCartesien(float centerX, float centerY, float radius, float angle);
glm::vec2 polarCartesien(const glm::vec2 &point, float angleRadians, float radius);
glm::vec2 findMiddlePoint(const glm::vec2 firstPoint, const glm::vec2 secondPoint);
void createLineWithQuads(const glm::vec3 &start, const glm::vec3 &end, float thickness,
                         std::vector<float> &linePointsContainer, std::vector<glm::vec2> &midPoints);

#endif