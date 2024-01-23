#include "Camera.hpp"

void initCamera(Camera &camera) {
    camera.position = glm::vec3(0,10,0);
    camera.aspectRatio = 1;
    camera.roll = 0;
    camera.pitch = glm::radians(-90.0f);
    camera.yaw = 0;
}

void updateCamera(Camera &camera, Input &input) {
    // Compute the camera's orientation as vectors.
    glm::vec3 ux = glm::vec3(glm::rotate(glm::mat4(1), camera.yaw, glm::vec3(0,1,0)) * glm::vec4(1,0,0,1));
    glm::vec3 uy = glm::vec3(0,1,0);
    glm::vec3 uz = glm::vec3(glm::rotate(glm::mat4(1), camera.yaw, glm::vec3(0,1,0)) * glm::vec4(0,0,-1,1));
    
    // Update the camera's position.
    camera.position += ux * input.m.x * CAMERA_MOVE_SPEED * input.deltaT;
    camera.position += uy * input.m.y * CAMERA_MOVE_SPEED * input.deltaT;
    camera.position += uz * input.m.z * CAMERA_MOVE_SPEED * input.deltaT;
    
    // Update the camera's orientation
    camera.roll += input.r.z * CAMERA_ROTATE_SPEED * input.deltaT;
    // DISABLED FOR NOW.
    // camera.pitch -= input.r.x * CAMERA_ROTATE_SPEED * input.deltaT;
    camera.yaw -= input.r.y * CAMERA_ROTATE_SPEED * input.deltaT;
}

glm::mat4 projection(Camera &camera) {
    // perspective projection.
    float tang = tan(CAMERA_FOVy / 2);
    return glm::mat4(
        1 / (camera.aspectRatio * tang), 0, 0, 0,
        0, -1 / tang, 0, 0,
        0, 0, CAMERA_farPlane / (CAMERA_nearPlane - CAMERA_farPlane), -1,
        0, 0, CAMERA_nearPlane * CAMERA_farPlane / (CAMERA_nearPlane - CAMERA_farPlane), 0);
}

glm::mat4 lookInDirection( float Alpha, float Beta, float Rho, glm::vec3 Pos) {
    return glm::rotate(glm::mat4(1.0), -Rho, glm::vec3(0,0,1)) 
            * glm::rotate(glm::mat4(1.0), -Beta, glm::vec3(1,0,0))
            * glm::rotate(glm::mat4(1.0), -Alpha, glm::vec3(0,1,0))
            * glm::translate(glm::mat4(1.0), -Pos);
}

glm::mat4 view(Camera &camera) {
    return lookInDirection(camera.yaw, camera.pitch, camera.roll, camera.position);
}

glm::mat4 computeViewProjectionMatrix(Camera &camera) {
    return projection(camera) * view(camera);
}
