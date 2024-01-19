#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const int NUM_BALLS = 3;
const float BALL_HEIGHT = 1.55;
const float BALL_SCALE = 0.2;
struct Ball
{
    glm::vec2 position;
    
    glm::mat4 computeWorldMatrix() {
        return glm::translate(glm::mat4(1), glm::vec3(position.x, BALL_HEIGHT, position.y)) * glm::scale(glm::mat4(1), glm::vec3(BALL_SCALE));
    }
};

class GameLogic {
public:
    void initBalls();
    Ball getBall(int index) {return balls[index];}
private:
    Ball balls[NUM_BALLS];
};

