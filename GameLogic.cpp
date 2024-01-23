#include "GameLogic.hpp"
#include <iostream>

/*
    In the logical plane, every ball has a diameter of 1 unit.
    The table is 20x10, occupying the space between (-5, -10) and (5, 10).
    A ball's position denotes the position of its centre.
 */
const float TABLE_BOTTOM_EDGE = -10;
const float TABLE_TOP_EDGE = 10;
const float TABLE_LEFT_EDGE = -5;
const float TABLE_RIGHT_EDGE = 5;

void GameLogic::initBalls() {
    balls[0].position = glm::vec2(-2, 1);
    balls[1].position = glm::vec2(5, 4);
    balls[2].position = glm::vec2(2, 0);
}

void GameLogic::updateGame(Input input) {
    if(aiming) {
        if(!charging) {
            if(input.fire) { // started charging.
                chargeTime = 0.0f;
                charging = true;
            } else { // picking direction.
                std::cout << input.r.x;
                direction +=  input.r.x + input.deltaT * ROTATE_SPEED;
                if (direction > 360.0f)
                    direction -= 360.0f;
                if(direction < 0.0f)
                    direction += 360.0f;
            }
        }
        if(charging) {
            if(input.fire) { // still charging
                chargeTime += input.deltaT;
            } else {    // released
                charging = false;
                std::cout << chargeTime;
                balls[0].velocity = glm::vec2(cos(glm::radians(direction)), sin(glm::radians(direction))) * chargeTime * HIT_STRENGTH;
                aiming = false;
            }
        }
    } else {
        computeFrame(input.deltaT);
    }
}

void handleBallCollision(Ball& b1, Ball&b2) {
    glm::vec2 collision_vector = b2.position - b1.position;
    float correction = (b1.radius + b2.radius - glm::length(collision_vector)) / 2.0;
    glm::vec2 normal = glm::normalize(collision_vector);
    b1.position += -correction * normal;
    b2.position += correction * normal;
    
    glm::vec2 tangent = glm::vec2(-normal.y, normal.x);

    auto newB1Velocity = normal * (glm::dot(normal, b2.velocity)) + tangent * glm::dot(tangent, b1.velocity);
    auto newB2Velocity = normal * (glm::dot(normal, b1.velocity)) + tangent * glm::dot(tangent, b2.velocity);
    
    b1.velocity = newB1Velocity;
    b2.velocity = newB2Velocity;
}

void GameLogic::computeFrame(float deltaT) {
    
    // check edge collisions
    for(auto &ball : balls) {
        if(ball.position.y + ball.radius >= TABLE_RIGHT_EDGE) {
            ball.position.y = TABLE_RIGHT_EDGE - ball.radius;
            ball.velocity.y = -ball.velocity.y;
        }
        if(ball.position.y - ball.radius <= TABLE_LEFT_EDGE) {
            ball.position.y = TABLE_LEFT_EDGE + ball.radius;
            ball.velocity.y = -ball.velocity.y;
        }
        if(ball.position.x + ball.radius >= TABLE_TOP_EDGE) {
            ball.position.x = TABLE_TOP_EDGE - ball.radius;
            ball.velocity.x = -ball.velocity.x;
        }
        if(ball.position.x - ball.radius <= TABLE_BOTTOM_EDGE) {
            ball.position.x = TABLE_BOTTOM_EDGE + ball.radius;
            ball.velocity.x = -ball.velocity.x;
        }
    }
    
    // check collisions with other balls
    for( int i = 0; i < NUM_BALLS - 1; i++) {
        for( int j = i + 1; j < NUM_BALLS; j++) {
            Ball& b1 = balls[i];
            Ball& b2 = balls[j];
            if( glm::distance(b1.position, b2.position) < b1.radius + b2.radius) {
                handleBallCollision(b1, b2);
            }
        }
    }
    
    // apply friction
    for (auto &ball : balls) {
        if(glm::length(ball.velocity) > 0){
            auto deltaV = glm::normalize(ball.velocity) * FRICTION_FACTOR * deltaT;
            auto newVelocity = ball.velocity - deltaV;
            // if the sign has changed.
            if(ball.velocity.x * newVelocity.x < 0.0f || ball.velocity.y * newVelocity.y < 0.0f)
                ball.velocity = glm::vec2(0);
            else
                
                ball.velocity = newVelocity;
        }
    }
        
    // apply displacement
    for (auto &ball : balls) {
        ball.position += deltaT * ball.velocity;
    }
    
}

glm::mat4 GameLogic::computeArrowWorldMatrix() {
    return balls[0].computeTranslationMatrix() // move it next to the ball.
    * glm::rotate(glm::mat4(1), glm::radians(direction), glm::vec3(0,1,0)) // rotate around origin
    * glm::scale(glm::mat4(1), glm::vec3(1.0f + fmaxf(0, chargeTime * ARROW_ELONGATE_FACTOR), 1, 1)) // scale horizontally based on charge time.
    * glm::translate(glm::mat4(1), glm::vec3(-ARROW_DISTANCE, 0, 0))    // move it away from the origin
    * glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0,1,0)); // orient the arrow horizontally.
}

// --------- Testing
void GameLogic::setRandomBallVelocities() {
    for (auto &ball : balls) {
        ball.velocity = glm::vec2(rand() % 4 - 2 , rand() % 4 - 2) * 3.0f;
    }
}
