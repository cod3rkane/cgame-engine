#include <stdio.h>
#include "external/glad.h"
#include <GLFW/glfw3.h>
#include "cod3rGL.h"
#include "external/timer.h"

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "CGame - Learn OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    glViewport(0, 0, 1280, 720);

    Shader defaultShader = LoadShader("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");
    glGenVertexArrays(1, &currentVaoId);

    Vector4 blue = { 0.219608f, 0.619608f, 0.909804f, 1.0f };
    Mesh mesh = createRect(&blue);

    timer_t timer;
    timer_start(&timer);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(defaultShader.id);
        glBindVertexArray(currentVaoId);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glDisable(GL_BLEND);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    printf("Running time in seconds: %ld\n", timer_delta_s(&timer));
    printf("Running time in ms: %ld\n", timer_delta_ms(&timer));
    timer_pause(&timer);

    glfwTerminate();
    return 0;
}
