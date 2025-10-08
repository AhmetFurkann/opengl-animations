#include "utils.h"
#include <iostream>


std::string glmToText(glm::vec3 point, uint precisionVal){
  std::string pointXtext = std::to_string(point.x).substr(0, std::to_string(point.x).find(".") + precisionVal + 1);
  std::string pointYtext = std::to_string(point.y).substr(0, std::to_string(point.y).find(".") + precisionVal + 1);
  // Not added the Z dim for presentation purposes
  //std::string pointYtext = std::to_string(point.y).substr(0, std::to_string(point.y).find(".") + precisionVal + 1);
  return "(" + pointXtext + ", " + pointYtext + ")";
}

float normalize_value(float value, float r_min, float r_max, float t_min, float t_max)
{
  float normalized_value = (value - r_min) / (r_max - r_min) * (t_max - t_min) + t_min;
  return normalized_value;
}

float degreeToRad(float angle)
{
  return angle * M_PI / 180;
}

Point polartToCartesien(float centerX, float centerY, float radius, float angle)
{
  float radian = degreeToRad(angle);
  Point point;
  std::cout << "DEBUG Radian: " << radian << std::endl;
  point.x = centerX + radius * cos(radian);
  point.y = centerY + radius * sin(radian);
  return point;
}

glm::vec2 polarCartesien(const glm::vec2 &point, float angleRadians, float radius)
{
  float x = point.x + radius * std::cos(angleRadians);
  float y = point.y + radius * std::sin(angleRadians);
  return glm::vec2(x, y);
}

glm::vec2 findMiddlePoint(const glm::vec2 firstPoint, const glm::vec2 secondPoint){
  glm::vec2 ret;
  ret.x = (firstPoint.x + secondPoint.x) / 2;
  ret.y = (firstPoint.y + secondPoint.y) / 2;
  return ret;
}

void createLineWithQuads(const glm::vec3 &start,
                         const glm::vec3 &end,
                         //const glm::vec3 &color,
                         float thickness,
                         std::vector<float> &linePointsContainer,
                         std::vector<glm::vec2> &midPoints)
{
  /*

               TL*    end   *TR
                  |‾‾‾ | ‾‾‾|
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |  
                  |    |    |
                  |___ | ___|
               BL*   start    *BR



               /‾‾‾‾‾/‾‾‾‾‾/
              /     /     /
             /     /     / 
            /     /     /
           /     /     /

  start: glm::vec3 &start;
  end  : glm::vec3 &end;
  TL   : Top-Left
  TR   : Top-Right
  BL   : Bottom-Left
  BR   : Bottom-Right


  TR.x = start.x * cos(90)
  */

  thickness /= 2;
  glm::vec2 delta = end - start;

  // Angle in radians relative to +X axis
  float angleRad = std::atan2(delta.y, delta.x);

  // Convert to degrees if needed:
  float angleDeg = glm::degrees(angleRad);

  //TODO: Check one more!
  float bottomRightDegree = angleDeg - 90.0f;
  float bottomLeftDegree = angleDeg + 90.0f;

  float bottomRightRadian = degreeToRad(bottomRightDegree);
  float bottomLeftRadian = degreeToRad(bottomLeftDegree);

  glm::vec3 bottomRight {polarCartesien(start, bottomRightRadian, thickness), 0.0f};
  glm::vec3 bottomLeft {polarCartesien(start, bottomLeftRadian, thickness), 0.0f};

  float topRightDegree = angleDeg - 90.0f;
  float topLeftDegree = angleDeg + 90.0f;

  float topRightRadian = degreeToRad(topRightDegree);
  float topLeftRadian = degreeToRad(topLeftDegree);

  glm::vec3 topRight {polarCartesien(end, topRightRadian, thickness), 0.0f};
  glm::vec3 topLeft {polarCartesien(end, topLeftRadian, thickness), 0.0f};

  /* TEXTURE COORDS
  Texture coords generally follows [0,1] range.
               0,1           1,1                 
                TL*   end   *TR
                  |‾‾‾ | ‾‾‾|
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |
                  |    |    |  
                  |    |    |
                  |___ | ___|
               BL*   start   *BR
               0,0           1,0 

  Bottom Right: 1,0
  Bottom Left : 0,0
  Top Right   : 1,1
  Top Left    : 0,1
  */

  // Creating Quad Line with Two Triangle
  // First Triangle//
  linePointsContainer.push_back(bottomRight.x);
  linePointsContainer.push_back(bottomRight.y);
  linePointsContainer.push_back(bottomRight.z);
  // Texture Coords
  linePointsContainer.push_back(1.0f);
  linePointsContainer.push_back(0.0f);

  linePointsContainer.push_back(topRight.x);
  linePointsContainer.push_back(topRight.y);
  linePointsContainer.push_back(topRight.z);
  // Texture Coords
  linePointsContainer.push_back(1.0f);
  linePointsContainer.push_back(1.0f);

  linePointsContainer.push_back(topLeft.x);
  linePointsContainer.push_back(topLeft.y);
  linePointsContainer.push_back(topLeft.z);
  // Texture Coords
  linePointsContainer.push_back(0.0f);
  linePointsContainer.push_back(1.0f);

  // Second Triangle //
  linePointsContainer.push_back(bottomRight.x);
  linePointsContainer.push_back(bottomRight.y);
  linePointsContainer.push_back(bottomRight.z);
  // Texture Coords
  linePointsContainer.push_back(1.0f);
  linePointsContainer.push_back(0.0f);

  linePointsContainer.push_back(bottomLeft.x);
  linePointsContainer.push_back(bottomLeft.y);
  linePointsContainer.push_back(bottomLeft.z);
  // Texture Coords
  linePointsContainer.push_back(0.0f);
  linePointsContainer.push_back(0.0f);

  linePointsContainer.push_back(topLeft.x);
  linePointsContainer.push_back(topLeft.y);
  linePointsContainer.push_back(topLeft.z);
  // Texture Coords
  linePointsContainer.push_back(0.0f);
  linePointsContainer.push_back(1.0f);

  glm::vec2 middlePoint = findMiddlePoint(topLeft, bottomRight);
  midPoints.push_back(middlePoint);
}