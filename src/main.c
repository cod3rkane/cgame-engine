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

    Mesh mesh;
    mesh.vertices = (float *)malloc( 12 * sizeof(float));
    mesh.colors = (float *)malloc( 16 * sizeof(float));
    mesh.indices = (unsigned int *)malloc(6 * sizeof(unsigned int));
    mesh.vboId = (unsigned int *)malloc(3 * sizeof(unsigned int *));
    
    mesh.vboId[0] = 0; // positions
    mesh.vboId[1] = 0; // colors
    mesh.vboId[2] = 0; // indices

    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };

    float colors[] = {
        0.368627f, 1.0f, 0.737255f, 0.0f,
        0.368627f, 1.0f, 0.737255f, 0.0f,
        0.368627f, 1.0f, 0.737255f, 1.0f,
        0.368627f, 1.0f, 0.737255f, 1.0f
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.colors = colors;

    glGenBuffers(1, &mesh.vboId[0]);
    glGenBuffers(1, &mesh.vboId[1]);
    glGenBuffers(1, &mesh.vboId[2]);

    glBindVertexArray(currentVaoId);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), mesh.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(defaultShader.locs[LOC_VERTEX_POSITION], 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(defaultShader.locs[LOC_VERTEX_POSITION]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[1]);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), mesh.colors, GL_STATIC_DRAW);

    glVertexAttribPointer(defaultShader.locs[LOC_VERTEX_COLOR], 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(defaultShader.locs[LOC_VERTEX_COLOR]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vboId[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), mesh.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

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
