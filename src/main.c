#include <stdio.h>
#include "external/glad.h"
#include <GLFW/glfw3.h>
#include "cod3rGL.h"
#include "external/timer.h"
#include <cglm/cglm.h>

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

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "CGame - Learn OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    InitCod3rGL(windowWidth, windowHeight);

    Vector4 blue = { 0.219608f, 0.619608f, 0.909804f, 1.0f };
    Vector4 pink = { 0.901961f, 0.611765f, 1.0f };
    Vector4 purple = { 0.721569f, 0.556863f, 0.909804f };
    Vector4 magenta = { 0.72549f, 0.658824f, 1.0f };

    Mesh mesh = CreateRect(&blue, (vec3){ 300.0f, 200.0f, 0.0f });
    Mesh mesh2 = CreateRect(NULL, (vec3){ -1.0f, 0.8f, 0.0f });

    timer_t timer;
    timer_start(&timer);

    glm_perspective(glm_rad(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f, projection);
    glm_translate(view, (vec3){0.0f, 0.0f, -3.0f});

    int frameBufferWidth, frameBufferHeight;

    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

        glfwPollEvents();

        glMatrixMode(GL_PROJECTION);
        glViewport(0, 0, frameBufferWidth, frameBufferHeight);
        glMatrixMode(GL_MODELVIEW);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        DrawRect(mesh);
        DrawRect(mesh2);

        RenderCod3rGL();

        glfwSwapBuffers(window);
    }

    printf("Running time in seconds: %ld\n", timer_delta_s(&timer));
    printf("Running time in ms: %ld\n", timer_delta_ms(&timer));
    timer_pause(&timer);

    CleanCod3rGL();
    glfwTerminate();
    return 0;
}
