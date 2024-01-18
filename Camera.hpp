#pragma once

#include "Input.hpp"

const float CAMERA_FOVy = glm::radians(90.0f);
const float CAMERA_nearPlane = 0.1f;
const float CAMERA_farPlane = 100.0f;
const float CAMERA_MOVE_SPEED = 2.0f;
const float CAMERA_ROTATE_SPEED = glm::radians(120.0f);
struct Camera {
    glm::vec3 position;
    float roll;
    float pitch;
    float yaw;
    float aspectRatio;
};
void updateCamera(Camera &camera, Input &input);
glm::mat4 computeViewProjectionMatrix(Camera &camera);
void initCamera(Camera &camera);
