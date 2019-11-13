#include <stdio.h>
#include "external/glad.h"
#include <GLFW/glfw3.h>
#include "cod3rGL.h"

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

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    mesh.vertices = vertices;
    mesh.indices = indices;

    glGenBuffers(1, &mesh.vboId[0]);
    glGenBuffers(1, &mesh.vboId[1]);

    glBindVertexArray(currentVaoId);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), mesh.vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vboId[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), mesh.indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0); 

    // glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[1]);
    // glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), mesh.colors, GL_STATIC_DRAW);
    // glVertexAttribPointer(defaultShader.locs[LOC_VERTEX_COLOR], 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), mesh.colors);
    // glEnableVertexAttribArray(defaultShader.locs[LOC_VERTEX_COLOR]);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(defaultShader.id);
        glBindVertexArray(currentVaoId);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
