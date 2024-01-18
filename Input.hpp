#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Input {
    float deltaT;
    glm::vec3 m = glm::vec3(0);
    glm::vec3 r = glm::vec3(0);
    bool fire;
};

