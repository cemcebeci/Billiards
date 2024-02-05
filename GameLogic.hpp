#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Input.hpp"

const int NUM_BALLS = 16;
const float BALL_HEIGHT = 0.0f;
const float BALL_SCALE = 0.4;
const float FRICTION_FACTOR = 1;
const float HIT_STRENGTH = 5.0f;
const float ROTATE_SPEED = 1.0f;
const float ARROW_DISTANCE = 1.5f;
const float ARROW_ELONGATE_FACTOR = 0.5f;

struct Hole
{
    glm::vec2 position;
    float radius = 1.5;
};

struct Ball
{
    int id = -1;
    glm::vec2 position;
    glm::mat4 rotation = glm::mat4(1);
    glm::vec2 velocity = glm::vec2(0);
    float radius = 1; // in logical units.
    Hole* inHole = nullptr;
    bool animatingFall = false;
    bool hide = false;
    
    glm::mat4 computeWorldMatrix() {
        if(hide)
            return glm::scale(glm::mat4(1), glm::vec3(0));
        
        return computeTranslationMatrix() * glm::mat4(rotation) * glm::scale(glm::mat4(1), glm::vec3(BALL_SCALE * radius));
    }
    
    glm::mat4 computeTranslationMatrix() {
        return glm::translate(glm::mat4(1), glm::vec3(position.x / 2, BALL_HEIGHT, -position.y / 2));
    }
};

class GameLogic {
public:
    void init() {initBalls(); initHoles();};
    Ball getBall(int index) {return balls[index];}
    void updateGame(Input input);
    glm::mat4 computeArrowWorldMatrix();
    glm::mat4 pointerWorldMatrix();
    int getCurrentPlayer() {return currentPlayer;}
    
    
private:
    Ball balls[NUM_BALLS];
    Hole holes[6];
    bool charging = false;
    bool aiming = true;
    float direction = 0.0f;
    float chargeTime;
    int currentPlayer = 0;
    
    void computeFrame(float deltaT);
    bool allBallsAreStill();
    void checkWhetherAnyBallsGoIn();
    void handleScore(Ball&, Hole&);
    
    void initBalls();
    void initHoles();
    
    // testing
    void setRandomBallVelocities();
};

