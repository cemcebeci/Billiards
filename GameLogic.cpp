#include "GameLogic.hpp"
#include <iostream>

/*
    In the logical plane, every ball has a diameter of 1 unit.
    The table is 20x10, occupying the space between (-5, -10) and (5, 10).
    A ball's position denotes the position of its centre.
 */
const float TABLE_BOTTOM_EDGE = -9.5;
const float TABLE_TOP_EDGE = 9.5;
const float TABLE_LEFT_EDGE = -16.5;
const float TABLE_RIGHT_EDGE = 16.5;

void GameLogic::initBalls() {
    for (int i = 0; i < NUM_BALLS; i++) {
        balls[i].id = i;
    }

    balls[0].position = glm::vec2(0,    -6);
    balls[1].position = glm::vec2(0,    1 + 0);
    balls[2].position = glm::vec2(1,    1 + sqrt(3));
    balls[3].position = glm::vec2(-1,   1 + sqrt(3));
    balls[4].position = glm::vec2(2,    1 + 2 * sqrt(3));
    balls[5].position = glm::vec2(0,    1 + 2 * sqrt(3));
    balls[6].position = glm::vec2(-2,   1 + 2 * sqrt(3));
    balls[7].position = glm::vec2(3,    1 + 3 * sqrt(3));
    balls[8].position = glm::vec2(1,    1 + 3 * sqrt(3));
    balls[9].position = glm::vec2(-1,   1 + 3 * sqrt(3));
    balls[10].position = glm::vec2(-3,  1 + 3 * sqrt(3));
    balls[11].position = glm::vec2(4,   1 + 4 * sqrt(3));
    balls[12].position = glm::vec2(2,   1 + 4 * sqrt(3));
    balls[13].position = glm::vec2(0,   1 + 4 * sqrt(3));
    balls[14].position = glm::vec2(-2,  1 + 4 * sqrt(3));
    balls[15].position = glm::vec2(-4,  1 + 4 * sqrt(3));
}

void GameLogic::initHoles() {
    holes[0].position = glm::vec2(-9.5, -16.5);
    holes[0].radius = 2;
    holes[1].position = glm::vec2(9.5, -16.5);
    holes[1].radius = 2;
    holes[2].position = glm::vec2(-9.5, 16.5);
    holes[2].radius = 2;
    holes[3].position = glm::vec2(9.5, 16.5);
    holes[3].radius = 2;
    holes[4].position = glm::vec2(-9.5, 0);
    holes[5].position = glm::vec2(9.5, 0);
}

bool GameLogic::allBallsAreStill() {
    for(auto &ball : balls) {
        if (ball.inHole != nullptr) {
            if(ball.animatingFall)
                return false;
        } else {
            if(ball.velocity.x != 0.0f || ball.velocity.y != 0.0f)
                return false;
        }
    }
    return true;
}

void GameLogic::handle8Pocket(int pocketingPlayer) {
    if(pocketingPlayer == 0) {
        for (int i = 0; i < NUM_BALLS; i++) {
            if(balls[i].getType() == p1Color && balls[i].inHole == nullptr) {
                winner = 1;
                return;
            }
        }
        winner = 0;
        return;
    }
    for (int i = 1; i < 8; i++) {
        if(balls[i].inHole == nullptr &&
           (balls[i].getType() != p1Color ||
            balls[i].getType() != Ball::CUE ||
            balls[i].getType() != Ball::EIGHT)
        ) {
            winner = 0;
            return;
        }
    }
    winner = 1;
    return;
}

void GameLogic::handleScore(Ball& ball, Hole& hole) {
    ball.inHole = &hole;
    ball.animatingFall = true;
    
    if (ball.getType() == Ball::CUE) {
        faultThisShot = true;
        return;
    }
    if (ball.getType() == Ball::EIGHT) {
        return;
    }
    
    if(!colorsChosen) {
        colorsChosen = true;
        if(currentPlayer == 0) {
            p1Color = ball.getType();
        } else {
            p1Color = (ball.getType() == Ball::FULL) ? Ball::STRIPE : Ball::FULL;
        }
    }
    
    if ((currentPlayer == 0 && ball.getType() == p1Color)
        || (currentPlayer == 1 && ball.getType() != p1Color)) {
        scoredThisShot = true;
    }
}

void GameLogic::checkWhetherAnyBallsGoIn() {
    for( auto &ball : balls) {
        if (ball.inHole != nullptr)
            continue;
        for( auto &hole : holes) {
            if (glm::distance(ball.position, hole.position) < hole.radius) {
                handleScore(ball, hole);
            }
        }
    }
}

void GameLogic::updateGame(Input input) {
    if(aiming && winner == -1) {
        if(!charging) {
            if(input.fire) { // started charging.
                chargeTime = 0.0f;
                charging = true;
            } else { // picking direction.
                direction +=  input.r.y * input.deltaT * ROTATE_SPEED;
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
                balls[0].velocity = glm::vec2(cos(glm::radians(direction)), sin(glm::radians(direction))) * chargeTime * HIT_STRENGTH;
                aiming = false;
                chargeTime = 0.0f;
            }
        }
    } else {
        checkWhetherAnyBallsGoIn();
        computeFrame(input.deltaT);
        if( allBallsAreStill() && winner == -1) {
            // finished pyhsics simulation
            if ( !scoredThisShot || faultThisShot) {
                currentPlayer = 1 - currentPlayer;
            }
            
            if(!touchedABallThisShot)
                faultThisShot = true;
            
            if( faultThisShot) {
                // TODO animate?
                balls[0].position = glm::vec2(0,    -6);
                balls[0].velocity = glm::vec2(0);
                balls[0].inHole = nullptr;
                balls[0].hide = false;
                checkCollisions();
            }
            
            aiming = true;
            scoredThisShot = false;
            faultThisShot = false;
            touchedABallThisShot = false;
            firstShot = false;
        }
    }
}

void GameLogic::handleBallCollision(Ball& b1, Ball&b2) {
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
    
    if(b1.getType() == Ball::CUE && !touchedABallThisShot) { // assuming CUE always has smaller id.
        touchedABallThisShot = true;
        if(colorsChosen) {
            if((currentPlayer == 0 && b2.getType() != p1Color)
               || (currentPlayer == 1 && (b2.getType() == p1Color || b2.getType() == Ball::EIGHT)))
                faultThisShot = true;
        }
    }
}

void GameLogic::applyAnimation(Ball& ball, float deltaT) {
    Hole hole = *ball.inHole;
    if(glm::distance(ball.position, hole.position) > 0.1) {
        glm::vec2 direction = glm::normalize( hole.position - ball.position);
        ball.position += direction * deltaT * glm::length(ball.velocity);
    } else {
        ball.animatingFall = false;
        ball.hide = true;
        if(ball.getType() == Ball::EIGHT) {
            handle8Pocket(currentPlayer);
            std::cout << "Winner: " << winner;
        }
    }
        
}

void GameLogic::checkCollisions() {
    // check edge collisions
    for(auto &ball : balls) {
        if(ball.inHole != nullptr)
            continue;
        
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
        Ball& b1 = balls[i];
        if(b1.hide)
            continue;
        for( int j = i + 1; j < NUM_BALLS; j++) {
            Ball& b2 = balls[j];
            if(b2.hide)
                continue;
            if(glm::distance(b1.position, b2.position) < b1.radius + b2.radius) {
                handleBallCollision(b1, b2);
            }
        }
    }
}

void GameLogic::computeFrame(float deltaT) {
    checkCollisions();
    
    // apply friction
    for (auto &ball : balls) {
        if(ball.inHole != nullptr)
            continue;
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
        if(ball.inHole != nullptr)
            continue;
        ball.position += deltaT * ball.velocity;
    }
    
    // apply animation
    for (auto &ball : balls) {
        if(ball.animatingFall)
            applyAnimation(ball, deltaT);
    }
    
    // apply rotation
    for(auto &ball : balls) {
        if(glm::length(ball.velocity) > 0) {
            auto axis = glm::vec3(ball.velocity.y, 0, ball.velocity.x);
            auto amount = glm::length(ball.velocity) * deltaT / ball.radius / 2;

                
            auto rotator = glm::rotate(glm::mat4(1), -amount, axis);

            ball.rotation = rotator * ball.rotation;
        }
    }
    
}

glm::mat4 GameLogic::computeStickWorldMatrix() {
    if(!aiming)
        return glm::scale(glm::mat4(1), glm::vec3(0,0,0));
    
    return balls[0].computeTranslationMatrix() // move it next to the ball.
    * glm::rotate(glm::mat4(1), glm::radians(direction), glm::vec3(0,1,0)) // rotate around origin
    * glm::translate(glm::mat4(1), glm::vec3(-(ARROW_DISTANCE + chargeTime * ARROW_ELONGATE_FACTOR), 0, 0))    // move it away from the origin
    * glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0,1,0)); // orient the arrow horizontally.
}

// --------- Testing
void GameLogic::setRandomBallVelocities() {
    for (auto &ball : balls) {
        ball.velocity = glm::vec2(rand() % 4 - 2 , rand() % 4 - 2) * 3.0f;
    }
}

glm::mat4 GameLogic::pointerWorldMatrix() {
    return glm::scale(glm::mat4(1), glm::vec3(0,0,0));
    
    auto hole = holes[1];
    
    return glm::translate(glm::mat4(1), glm::vec3(hole.position.x / 2, BALL_HEIGHT, -hole.position.y / 2))
        * glm::scale(glm::mat4(1), glm::vec3(BALL_SCALE * hole.radius));
}
