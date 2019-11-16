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
    Mesh mesh = CreateRect(&blue, (vec3){ 1.0f, 0.8f, 0.0f });
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

        // drawRect(mesh);
        // drawRect(mesh2);

        glBindVertexArray(currentVaoId); // current global VAO

        // append vertices
        for (int i = 0; i < 12; i++) {
            // 12 == length of vertices
            mesh.vertices[12 + i] = mesh2.vertices[i];
        }
        // append colors
        for (int i = 0; i < 16; i++) {
            // 12 == length of vertices
            mesh.colors[16 + i] = mesh2.colors[i];
        }
        // append indices
        for (int i = 0; i < 6; i++) {
            mesh.indices[6 + i] = mesh2.indices[i] + 3 + 1;
        }

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[0]);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), mesh.vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(LOC_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(LOC_VERTEX_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[1]);
        glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(float), mesh.colors, GL_STATIC_DRAW);

        glVertexAttribPointer(LOC_VERTEX_COLOR, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glEnableVertexAttribArray(LOC_VERTEX_COLOR);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentIndexBuffer.bufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * sizeof(unsigned int), mesh.indices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

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
