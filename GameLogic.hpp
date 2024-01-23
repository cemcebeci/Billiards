#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Input.hpp"

const int NUM_BALLS = 3;
const float BALL_HEIGHT = 1.35;
const float BALL_SCALE = 0.5;
const float FRICTION_FACTOR = 0.2;
const float HIT_STRENGTH = 5.0f;
const float ROTATE_SPEED = 1.0f;
const float ARROW_DISTANCE = 1.5f;
const float ARROW_ELONGATE_FACTOR = 0.5f;
struct Ball
{
    glm::vec2 position;
    glm::vec2 velocity = glm::vec2(0);
    float radius = 1; // in logical units.
    
    glm::mat4 computeWorldMatrix() {
        return computeTranslationMatrix() * glm::scale(glm::mat4(1), glm::vec3(BALL_SCALE * radius));
    }
    
    glm::mat4 computeTranslationMatrix() {
        return glm::translate(glm::mat4(1), glm::vec3(position.x / 2, BALL_HEIGHT, -position.y / 2));
    }
};

class GameLogic {
public:
    void initBalls();
    Ball getBall(int index) {return balls[index];}
    void updateGame(Input input);
    glm::mat4 computeArrowWorldMatrix();
    
    
private:
    Ball balls[NUM_BALLS];
    bool charging = false;
    bool aiming = true;
    float direction = 0.0f;
    float chargeTime;
    void computeFrame(float deltaT);
    
    // testing
    void setRandomBallVelocities();
};

