#pragma once
#ifndef ARCBALL_H
#define ARCBALL_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Arcball {
public:
    bool isDragging;
    Arcball() : isDragging(false) {}
    glm::quat rotationQuat;

    // Call this function when the mouse is pressed (button down)
    void onMouseDown(const glm::vec2& mousePos) {
        isDragging = true;
        startDragPos = normalizeMousePos(mousePos);
        lastDragPos = startDragPos;
    }

    // Call this function when the mouse is released (button up)
    void onMouseUp() {
        isDragging = false;
    }

    // Call this function when the mouse is moved (during dragging)
    void onMouseMove(const glm::vec2& mousePos, float sensitivity = 1.001f) {
        if (isDragging) {
            //cout << "Êó±ê¶¯ÁË" << endl;
            glm::vec3 currentDragPos = normalizeMousePos(mousePos);
            
            // Calculate rotation axis (perpendicular to start and end drag positions)
            glm::vec3 axis = glm::cross(startDragPos, currentDragPos);
            
            // Calculate the angle of rotation
            float dotProduct = glm::dot(startDragPos, currentDragPos);
            
            float angle = glm::acos(glm::min(1.0f, dotProduct)) * 2.0f;
            
            // Create quaternion for the rotation
            glm::quat rotation = glm::angleAxis(angle* sensitivity, glm::normalize(axis));

            // Update last drag position
            lastDragPos = currentDragPos;

            rotationQuat= rotation;
  
            //std::cout << "Quaternion: (" << rotationQuat.x << ", " << rotationQuat.y << ", " << rotationQuat.z << ", " << rotationQuat.w << ")" << std::endl;
        } else {
            rotationQuat =  glm::quat(); // No rotation when not dragging
        }
    }

private:

    glm::vec3 startDragPos;
    glm::vec3 lastDragPos;
 
    // Normalize mouse position to a unit sphere
    glm::vec3 normalizeMousePos(const glm::vec2& mousePos) {
        glm::vec2 normalizedPos = mousePos / glm::vec2(1600.0f, 900.0f) * 2.0f - 1.0f;
        normalizedPos.y = -normalizedPos.y; // Invert y-axis

        float lengthSquared = glm::dot(normalizedPos, normalizedPos);
        if (lengthSquared <= 1.0f) {
            float z = glm::sqrt(1.0f - lengthSquared);
            return glm::vec3(normalizedPos.x, normalizedPos.y, z);
        }
        else {
            return glm::normalize(glm::vec3(normalizedPos.x, normalizedPos.y, 0.0f));
        }
    }
};

#endif
