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
        camera->position += cameraSpeed * camera->front;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->position -= cameraSpeed * camera->front;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->position -= glm::normalize(glm::cross(camera->front, camera->up)) * cameraSpeed;
    }
    
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->position += glm::normalize(glm::cross(camera->front, camera->up)) * cameraSpeed;
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
        MouseMovementCamera(xoffset, yoffset, false);
    }

    lastMouseX = mouseX;
    lastMouseY = mouseY;
}
