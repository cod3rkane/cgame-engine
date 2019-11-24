#include <iostream>
#include <stdio.h>
#include "external/glad.h"
#include <GLFW/glfw3.h>

#define COD3R_GL_IMPLEMENTATION
#include "cod3rGL.h"
#include <glm/vec3.hpp>
#include "interactions.h"

int windowWidth = 1280;
int windowHeight = 720;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "CGame - Learn OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    InitCod3rGL(windowWidth, windowHeight);

    int frameBufferWidth, frameBufferHeight;

    SetupCamera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

    Vector4 blue = {0.219608f, 0.619608f, 0.909804f, 1.0f};
    Vector4 pink = {0.901961f, 0.611765f, 1.0f, 1.0f};
    Vector4 purple = {0.721569f, 0.556863f, 0.909804f, 1.0f};
    Vector4 magenta = {0.72549f, 0.658824f, 1.0f, 1.0f};

    Entity testeMeshA = CreateRect(&purple, glm::vec3(0.0f, 0.0f, 0.0f));
    Entity testeMeshB = CreateRect(&blue, glm::vec3(250.0f, 150.0f, 0.0f));
    Entity testeMeshC = CreateRect(&magenta, glm::vec3(0.0f, 0.0f, 0.0f));

    Buffer buffer = CreateBuffer(BufferRenderType::Elements);
    StoreBuffer(&buffer);

    Entity terrain = CreateTerrain(glm::vec3(-100.0f, -100.0f, 0.0f));

    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

        glMatrixMode(GL_PROJECTION);
        glViewport(0, 0, frameBufferWidth, frameBufferHeight);
        glMatrixMode(GL_MODELVIEW);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);

        UserInputs(window, 0.05f, &currentCamera);

        RotateEntityZ(&testeMeshA, 0.4f);
        RotateEntityZ(&testeMeshB, 0.5f);
        RotateEntityZ(&testeMeshC, 1.0f);

        DrawEntity(testeMeshA);
        DrawEntity(testeMeshB);
        DrawEntity(testeMeshC);

        BindBuffer(buffer.id);
        DrawEntity(terrain);

        RenderCod3rGL();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    CleanCod3rGL();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
