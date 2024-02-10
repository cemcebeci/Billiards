#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Input.hpp"

const int NUM_BALLS = 16;
const float BALL_HEIGHT = 0.5f;
const float BALL_SCALE = 0.4;
const float FRICTION_FACTOR = 1;
const float HIT_STRENGTH = 7.5f;
const float ROTATE_SPEED = 90.0f;
const float ARROW_DISTANCE = 0.5f;
const float ARROW_ELONGATE_FACTOR = 1.0f;

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
    
    enum BallType {CUE, EIGHT, FULL, STRIPE};

    BallType getType() {
        if(id == 0) return CUE;
        if(id < 8) return FULL;
        if(id == 8) return EIGHT;
        return STRIPE;
    }
    
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
    glm::mat4 computeStickWorldMatrix();
    glm::mat4 pointerWorldMatrix();
    int getCurrentPlayer() {return currentPlayer;}
    Ball::BallType getTargetType() {
        if(!colorsChosen) return Ball::CUE;
        if(currentPlayer == 0) return p1Color;
        else return (p1Color == Ball::FULL) ? Ball::STRIPE : Ball::FULL;
    }
    int getWinner() {return winner;}
    
    float direction = 90.0f;
    bool aiming = true;
    Ball::BallType p1Color;
    bool colorsChosen = false;
    
    
private:
    Ball balls[NUM_BALLS];
    Hole holes[6];
    bool charging = false;
    float chargeTime = 0.0f;
    int currentPlayer = 0;
    bool scoredThisShot = false;
    bool faultThisShot = false;
    bool touchedABallThisShot = false;
    int winner = -1;
    bool firstShot = true;
    
    void computeFrame(float deltaT);
    bool allBallsAreStill();
    void checkWhetherAnyBallsGoIn();
    void handleScore(Ball&, Hole&);
    void handleBallCollision(Ball& b1, Ball&b2);
    void handle8Pocket(int pocketingPlayer);
    void applyAnimation(Ball& ball, float deltaT);
    void checkCollisions();
    
    void initBalls();
    void initHoles();
    
    // testing
    void setRandomBallVelocities();
};

