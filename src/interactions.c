#include "interactions.h"

bool firstMouse = true;
double lastMouseX;
double lastMouseY;

void UserInputs(GLFWwindow *window, float deltaTime, Camera *camera) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        // @TODO: change this...
        glfwSetWindowShouldClose(window, true);
    }

    float cameraSpeed = 2.5f * deltaTime;
    // Camera Actions
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        // position += front * speed
        glm_vec3_muladds(camera->front, cameraSpeed, camera->position);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 speed;
        glm_vec3_muladds(camera->front, cameraSpeed, speed);
        // camera position = pos - speed
        glm_vec3_sub(camera->position, speed, camera->position);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 crossed;
        glm_cross(camera->front, camera->up, crossed);
        glm_vec3_normalize(crossed);
        glm_vec3_adds(crossed, cameraSpeed, crossed);
        glm_vec3_sub(camera->position, crossed, camera->position);
    }
    
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 crossed;
        glm_cross(camera->front, camera->up, crossed);
        glm_vec3_normalize(crossed);
        glm_vec3_muladds(crossed, cameraSpeed, camera->position);
    }

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int mouseLeftBtn = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (mouseLeftBtn == GLFW_PRESS) {
        if (firstMouse) {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
        }

        float xoffset = mouseX - lastMouseX;
        float yoffset = lastMouseY - mouseY;
        MouseMovementCamera(xoffset, yoffset, true);
    }

    lastMouseX = mouseX;
    lastMouseY = mouseY;
}
