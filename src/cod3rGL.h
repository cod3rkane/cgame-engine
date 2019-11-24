#ifndef COD3R_GL_H
#define COD3R_GL_H

#include <stdio.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "external/glad.h"

#define DEFAULT_ATTRIB_POSITION_NAME "vertexPosition"
#define DEFAULT_ATTRIB_COLOR_NAME "vertexColor"
#define MAX_SHADER_LOCATIONS 32      // Maximum number of predefined locations stored in shader struct
#define MAX_DYNAMIC_DATA_PER_BUFFER 50000 // Maximum number of items per Dynamic Buffer
#define MAX_BUFFERS_RENDER 5 // Maximum number of buffers (VAO, VBOs)

// Structs
typedef struct Shader {
    unsigned int id;    // Shader Program ID
    int *locs;          // Shader locations array
} Shader;

typedef enum {
    LOC_VERTEX_POSITION = 0,
    LOC_VERTEX_COLOR = 1,
    LOC_MATRIX_PROJECTION,
    LOC_MATRIX_VIEW,
    LOC_MATRIX_MODEL,
} ShaderLocationIndex;

typedef struct Mesh {
    int vertexCount;            // number of vertices stored in arrays
    int triangleCount;          // number of triangles stored (indexed or not)
    int indicesCount;           // number of indices stored
    float *vertices;            // vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;           // vertex texture coordinates (UV - 2 components per vertex)
    float *texcoords2;          // vertex second texture coordinates (useful for lightmaps)
    float *normals;             // vertex normals (XYZ - 3 components per vertex)
    float *tangents;            // vertex tangents (XYZW - 4 components per vertex)
    float *colors;              // vertex colors (RGBA - 4 components per vertex)
    int *indices;    // vertex indices (in case vertex data comes indexed)

    // Animation vertex data
    float *animVertices;    // Animated vertex positions (after bones transformations)
    float *animNormals;     // Animated normals (after bones transformations)
    int *boneIds;           // Vertex bone ids, up to 4 bones influence by vertex (skinning)
    float *boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning)

    // OpenGL identifiers
    unsigned int vaoId;     // OpenGL Vertex Array Object id
    unsigned int *vboId;    // OpenGL Vertex Buffer Objects id
} Mesh;

typedef struct Entity {
    glm::mat4 matrix; // Local transform matrix

    int meshCount; // Number of Meshes
    Mesh *meshes; // Array of meshes
} Entity;

typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} Vector4;

typedef struct DynamicIBuffer {
    int vertexCount;
    int triangleCount;
    unsigned int bufferId;
    int *data;
} DynamicIBuffer;

typedef struct DynamicFBuffer {
    int vertexCount;
    int triangleCount;
    unsigned int bufferId;
    float *data;
} DynamicFBuffer;

enum BufferRenderType { Arrays, Elements };

typedef struct Buffer {
  unsigned int vaoId;
  DynamicFBuffer verticesBuffer;
  DynamicFBuffer colorsBuffer;
  DynamicIBuffer indexBuffer;
  BufferRenderType type;
  int id;
} Buffer;

typedef struct BufferHandler {
  Buffer *buffers;
  int size;
  int currentBuffer;
} BufferHandler;

typedef struct Camera {
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    glm::mat4 matrix;
} Camera;

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Functions
Shader LoadShader(const char *vsFileName, const char *fsFileName);
Shader LoadShaderCode(const char *vsCode, const char *fsCode);
static unsigned int CompileShader(const char *shaderStr, int type);
static unsigned int LoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId);
void UnloadShader(Shader shader);
static void SetShaderDefaultLocations(Shader *shader);
char *LoadText(const char *fileName);

Entity CreateRect(Vector4 *color, glm::vec3 position);
void DrawRect(Mesh mesh);

void DrawEntity(Entity entity);
void RotateEntityZ(Entity *entity, float angle);

void InitCod3rGL(int windowWidth, int windowHeight); // Initialise all global variables and other setups.
void CleanCod3rGL();
void RenderCod3rGL();

void StoreDataToBufferf(DynamicFBuffer *buffer, float *data, int dataSize);
void StoreDataToBufferi(DynamicIBuffer *buffer, int *data, int dataSize, int numTriangles);

Buffer CreateBuffer(enum BufferRenderType type);
void StoreBuffer(Buffer *buffer);
void BindBuffer(int id);
int GetCurrentBuffer();
void CleanBuffer(int id);
void CleanAllBuffers();

// Camera Functions

void SetupCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch); // Setup camera default data
void UpdateCameraVectors(); // Update Camera vectors
void MouseMovementCamera(float xOffset, float yOffset, bool constraintPitch); // Update camera based on given arguments
glm::mat4 GetViewMatrixCamera(); // Get Camera matrix

// Terrain
Entity CreateTerrain(glm::vec3 position);

#endif // COD3R_GL_H

#if defined(COD3R_GL_IMPLEMENTATION)

// Global Variables

BufferHandler bufferHandler;

Shader defaultShader;

glm::mat4 projection = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};
glm::mat4 model = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

Camera currentCamera;

// Functions Implementations

Shader LoadShader(const char *vsFileName, const char *fsFileName) {
    Shader shader = { 0 };

    char *vShaderStr = NULL;
    char *fShaderStr = NULL;

    if (vsFileName != NULL) vShaderStr = LoadText(vsFileName);

    if (fsFileName != NULL) fShaderStr = LoadText(fsFileName);

    shader = LoadShaderCode(vShaderStr, fShaderStr);

    if (vShaderStr != NULL) free(vShaderStr);
    if (fShaderStr != NULL) free(fShaderStr);

    return shader;
}

char *LoadText(const char *fileName) {
    FILE *textFile = NULL;
    char *text = NULL;

    if (fileName != NULL) {
        textFile = fopen(fileName, "r");

        if (textFile != NULL) {
            fseek(textFile, 0, SEEK_END);
            int size = ftell(textFile);
            fseek(textFile, 0, SEEK_SET);

            if (size > 0) {
                text = (char *)malloc(size * sizeof(char));
                int count = fread(text, sizeof(char), size, textFile);
                text[count] = '\0';
            }

            fclose(textFile);
        } else {
            std::cout << fileName << " Text file could not be opened" << std::endl;
        }
    }

    return text;
}

Shader LoadShaderCode(const char *vsCode, const char *fsCode) {
    Shader shader = { 0 };
    shader.locs = (int *)malloc(MAX_SHADER_LOCATIONS * sizeof(int));

    unsigned int vertexShaderId = 0;
    unsigned int fragmentShaderId = 0;

    if (vsCode != NULL) vertexShaderId = CompileShader(vsCode, GL_VERTEX_SHADER);
    if (fsCode != NULL) fragmentShaderId = CompileShader(fsCode, GL_FRAGMENT_SHADER);

    shader.id = LoadShaderProgram(vertexShaderId, fragmentShaderId);

    if (shader.id == 0) std::cout << "Custom shader could not be" << std::endl;

    if (shader.id > 0) SetShaderDefaultLocations(&shader);

    int uniformCount = -1;

    for (int i = 0; i < uniformCount; i++) {
        int namelen = -1;
        int num = -1;
        char name[256];
        GLenum type = GL_ZERO;

        // get the name of the uniform
        glGetActiveUniform(shader.id, i, sizeof(name) - 1, &namelen, &num, &type, name);
        name[namelen] = 0;

        // get the location of the named uniform
        unsigned int location = glGetUniformLocation(shader.id, name);
        printf("[Shader ID: %i] Active uniform [%s] set at locatiom: %i\n", shader.id, name, location);
    }

    return shader;
}

static unsigned int CompileShader(const char *shaderStr, int type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderStr, NULL);

    GLint success = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success != GL_TRUE) {
        printf("[Shader ID: %i] Failed to compile shader...\n", shader);
        int maxLength = 0;
        int length;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        char log[maxLength];
        glGetShaderInfoLog(shader, maxLength, &length, log);
        printf("%s\n", log);
    }

    printf("[Shader ID: %i] Shader compiled successfully\n", shader);

    return shader;
}

static unsigned int LoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId) {
    unsigned int program = 0;
    GLint success = 0;
    program = glCreateProgram();

    glAttachShader(program, vShaderId);
    glAttachShader(program, fShaderId);

    glBindAttribLocation(program, 0, DEFAULT_ATTRIB_POSITION_NAME);
    glBindAttribLocation(program, 1, DEFAULT_ATTRIB_COLOR_NAME);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
        printf("[Program ID: %i] Failed to link shader program...\n", program);
        int maxLength = 0;
        int length;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        char log[maxLength];

        glGetProgramInfoLog(program, maxLength, &length, log);

        printf("%s", log);
    } else {
        printf("[Program ID: %i] Shader program loaded successfully\n", program);
    }

    return program;
}

void UnloadShader(Shader shader) {
    if (shader.id > 0) {
        glDeleteShader(shader.id);
        printf("[Program ID: %i] Unloaded shader program data\n", shader.id);
    }
}

static void SetShaderDefaultLocations(Shader *shader) {
    shader->locs[LOC_VERTEX_POSITION] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_POSITION_NAME);
    shader->locs[LOC_VERTEX_COLOR] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_COLOR_NAME);

    shader->locs[LOC_MATRIX_PROJECTION] = glGetUniformLocation(shader->id, "projection");
    shader->locs[LOC_MATRIX_VIEW] = glGetUniformLocation(shader->id, "view");
    shader->locs[LOC_MATRIX_MODEL] = glGetUniformLocation(shader->id, "model");
}

Entity CreateRect(Vector4 *color, glm::vec3 position) {
    Entity entity;
    entity.meshes = (Mesh *)malloc(sizeof(Mesh));
    // entity.matrix = (mat4 *)malloc(sizeof(mat4));

    Mesh mesh = { 0 };
    mesh.vertexCount = 12;
    mesh.triangleCount = 4;
    mesh.indicesCount = 6;

    mesh.vertices = (float *)malloc( 12 * sizeof(float));
    mesh.colors = (float *)malloc( 16 * sizeof(float));
    mesh.indices = (int *)malloc(6 * sizeof(unsigned int));

    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    for (int i = 0; i < 12; i++) {
        mesh.vertices[i] = vertices[i];
    }

    if (color == NULL) {
        // default white
        // color = malloc(sizeof(Vector4));
        color->x = 1.0f;
        color->y = 1.0f;
        color->z = 1.0f;
        color->w = 1.0f;
    }

    for (int i = 0; i < 4; i++) {
        mesh.colors[i * 4] = color->x;
        mesh.colors[i * 4 + 1] = color->y;
        mesh.colors[i * 4 + 2] = color->z;
        mesh.colors[i * 4 + 3] = color->w;
    }

    for (int i = 0; i < 6; i++) {
        mesh.indices[i] = indices[i];
    }

    glm::mat4 matrix = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    matrix = glm::translate(matrix, position);
    entity.matrix = matrix;

    entity.meshes[0] = mesh;
    entity.meshCount = 1;

    return entity;
}

void DrawRect(Mesh mesh) {
    // @TODO: transformations
  StoreDataToBufferf(&bufferHandler.buffers[bufferHandler.currentBuffer].verticesBuffer, mesh.vertices, 12);
  StoreDataToBufferf(&bufferHandler.buffers[bufferHandler.currentBuffer].colorsBuffer, mesh.colors, 16);
  StoreDataToBufferi(&bufferHandler.buffers[bufferHandler.currentBuffer].indexBuffer, mesh.indices, 6, 4);
}

void RenderCod3rGL() {
  // @TODO: 3D render
  // @TODO: 2D render
  // @TODO: Use bufferHandler
  glUseProgram(defaultShader.id);

  for (int i = 0; i < bufferHandler.size; i++) {
    glBindVertexArray(bufferHandler.buffers[i].vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, bufferHandler.buffers[i].verticesBuffer.bufferId);
    glBufferData(
                 GL_ARRAY_BUFFER,
                 bufferHandler.buffers[i].verticesBuffer.vertexCount * sizeof(float),
                 bufferHandler.buffers[i].verticesBuffer.data,
                 GL_STATIC_DRAW
                 );

    glVertexAttribPointer(LOC_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(LOC_VERTEX_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, bufferHandler.buffers[i].colorsBuffer.bufferId);
    glBufferData(
                 GL_ARRAY_BUFFER,
                 bufferHandler.buffers[i].colorsBuffer.vertexCount * sizeof(float),
                 bufferHandler.buffers[i].colorsBuffer.data,
                 GL_STATIC_DRAW
                 );

    glVertexAttribPointer(LOC_VERTEX_COLOR, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(LOC_VERTEX_COLOR);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferHandler.buffers[i].indexBuffer.bufferId);
    glBufferData(
                 GL_ELEMENT_ARRAY_BUFFER,
                 bufferHandler.buffers[i].indexBuffer.vertexCount * sizeof(unsigned int),
                 bufferHandler.buffers[i].indexBuffer.data,
                 GL_STATIC_DRAW
                 );

    if (defaultShader.locs[LOC_MATRIX_PROJECTION] != -1) {
        glUniformMatrix4fv(defaultShader.locs[LOC_MATRIX_PROJECTION], 1, GL_FALSE, glm::value_ptr(projection));
    }

    if (defaultShader.locs[LOC_MATRIX_VIEW] != -1) {
        glUniformMatrix4fv(defaultShader.locs[LOC_MATRIX_VIEW], 1, GL_FALSE, glm::value_ptr(GetViewMatrixCamera()));
    }

    if (defaultShader.locs[LOC_MATRIX_MODEL] != -1) {
        glUniformMatrix4fv(defaultShader.locs[LOC_MATRIX_MODEL], 1, GL_FALSE, glm::value_ptr(model));
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferHandler.buffers[i].indexBuffer.bufferId);

    glDrawElements(GL_TRIANGLES, bufferHandler.buffers[i].indexBuffer.vertexCount, GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);

    CleanBuffer(i);
  }

  glBindVertexArray(0);
  glUseProgram(0);
}

void InitCod3rGL(int windowWidth, int windowHeight) {
  // Initialise buffers
  bufferHandler.buffers = (Buffer *)malloc(MAX_BUFFERS_RENDER * sizeof(struct Buffer));
  Buffer buffer = CreateBuffer(BufferRenderType::Elements); // Creates default Buffer
  StoreBuffer(&buffer);

  defaultShader = LoadShader("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");

  // setup matrices
  projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
}

void CleanCod3rGL() {
  for (int i = 0; i < bufferHandler.size; i++) {
    glDeleteVertexArrays(1, &bufferHandler.buffers[i].vaoId);
    glDeleteBuffers(1, &bufferHandler.buffers[i].verticesBuffer.bufferId);
    glDeleteBuffers(1, &bufferHandler.buffers[i].colorsBuffer.bufferId);
    glDeleteBuffers(1, &bufferHandler.buffers[i].indexBuffer.bufferId);
  }
}

void StoreDataToBufferf(DynamicFBuffer *buffer, float *data, int dataSize) {
    for (int i = 0; i < dataSize; i++) {
        buffer->data[buffer->vertexCount + i] = data[i];
    }

    buffer->vertexCount += dataSize;
}

void StoreDataToBufferi(DynamicIBuffer *buffer, int *data, int dataSize, int numTriangles) {
    for (int i = 0; i < dataSize; i++) {
        buffer->data[buffer->vertexCount + i] = data[i] + buffer->triangleCount;
    }

    buffer->vertexCount += dataSize;
    buffer->triangleCount += numTriangles;
}

void DrawEntity(Entity entity) {
    for (int i = 0; i < entity.meshCount; i++) {
        // apply matrix to vertex
        float formattedVertex[entity.meshes[i].vertexCount];
        glm::vec4 newPos = { 0.0f, 0.0f, 0.0f, 0.0f };
        for (int vertIndex = 0; vertIndex < entity.meshes[i].vertexCount;) {
            // glm_mat4_mulv(entity.matrix, (vec4){entity.meshes[i].vertices[vertIndex], entity.meshes[i].vertices[vertIndex + 1], entity.meshes[i].vertices[vertIndex + 2], 0.01f}, newPos);
            newPos = entity.matrix * glm::vec4(entity.meshes[i].vertices[vertIndex], entity.meshes[i].vertices[vertIndex + 1], entity.meshes[i].vertices[vertIndex + 2], 0.01f);
            formattedVertex[vertIndex] = newPos[0];
            formattedVertex[vertIndex + 1] = newPos[1];
            formattedVertex[vertIndex + 2] = newPos[2];

            vertIndex++;
            vertIndex++;
            vertIndex++;
        }

        StoreDataToBufferf(
                           &bufferHandler.buffers[bufferHandler.currentBuffer].verticesBuffer,
                           formattedVertex,
                           entity.meshes[i].vertexCount
                           );
        StoreDataToBufferf(
                           &bufferHandler.buffers[bufferHandler.currentBuffer].colorsBuffer,
                           entity.meshes[i].colors,
                           entity.meshes[i].vertexCount + entity.meshes[i].triangleCount
                           );
        StoreDataToBufferi(
                           &bufferHandler.buffers[bufferHandler.currentBuffer].indexBuffer,
                           entity.meshes[i].indices,
                           entity.meshes[i].indicesCount,
                           entity.meshes[i].triangleCount
                           );
    }
}

void RotateEntityZ(Entity *entity, float angle) {
    glm::mat4 matrix = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    entity->matrix = glm::rotate(entity->matrix, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
}

void SetupCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) {
    currentCamera.position = position;
    currentCamera.worldUp = up;

    currentCamera.yaw = yaw;
    currentCamera.pitch = pitch;

    currentCamera.front = glm::vec3(0.0f, 0.0f, -1.0f);
    currentCamera.movementSpeed = 2.5f;
    currentCamera.mouseSensitivity = 0.1f;
    currentCamera.zoom = 45.0f;

    UpdateCameraVectors();
}

void UpdateCameraVectors() {
    glm::vec3 front;

    front.x = std::cos(glm::radians(currentCamera.yaw)) * std::cos(glm::radians(currentCamera.pitch));
    front.y = std::sin(glm::radians(currentCamera.pitch));
    front.z = std::sin(glm::radians(currentCamera.yaw)) * std::cos(glm::radians(currentCamera.pitch));

    currentCamera.front = glm::normalize(currentCamera.front);
    currentCamera.right = glm::normalize(glm::cross(currentCamera.front, currentCamera.worldUp));
    currentCamera.up = glm::normalize(glm::cross(currentCamera.right, currentCamera.front));
}

void MouseMovementCamera(float xOffset, float yOffset, bool constraintPitch) {
    const float SENSITIVITY = 0.1f;
    xOffset *= SENSITIVITY;
    yOffset *= SENSITIVITY;

    currentCamera.yaw += xOffset;
    currentCamera.pitch += yOffset;

    if (constraintPitch) {
        if (currentCamera.pitch > 89.0f) {
            currentCamera.pitch = 89.0f;
        }

        if (currentCamera.pitch < -89.0f) {
            currentCamera.pitch = -89.0f;
        }
    }

    UpdateCameraVectors();
}

glm::mat4 GetViewMatrixCamera() {
    return glm::lookAt(currentCamera.position, currentCamera.position + currentCamera.front, currentCamera.up);
}

Entity CreateTerrain(glm::vec3 position) {
    Entity entity;
    entity.meshes = (Mesh *)malloc(sizeof(Mesh));

    glm::mat4 matrix = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    entity.matrix = glm::translate(matrix, position);

    // 10x10 grid size
    const float SIZE = 40.0f;
    const int VERTEX_COUNT = 4;
    Mesh mesh;
    mesh.vertices = (float *)malloc((VERTEX_COUNT * VERTEX_COUNT * 3) * sizeof(float));
    mesh.colors = (float *)malloc((VERTEX_COUNT * VERTEX_COUNT * 4) * sizeof(float));
    mesh.indices = (int *)malloc(((VERTEX_COUNT - 1) * (VERTEX_COUNT - 1) * 6) * sizeof(int));

    for (int z = 0; z < VERTEX_COUNT; z++) {
        for (int x = 0; x < VERTEX_COUNT; x++) {
            float xVert = (float) x / ((float)VERTEX_COUNT - 1) * SIZE;
            float zVert = (float) z / ((float)VERTEX_COUNT - 1) * SIZE;

            mesh.vertices[mesh.vertexCount] = xVert;
            mesh.vertexCount++;
            mesh.vertices[mesh.vertexCount] = 0.0f;
            mesh.vertexCount++;
            mesh.vertices[mesh.vertexCount] = zVert;
            mesh.vertexCount++;
        }
    }

    mesh.triangleCount = mesh.vertexCount / 3;
    const int MAX_COLORS = mesh.vertexCount + mesh.triangleCount;

    for (int i = 0; i < MAX_COLORS;) {
        mesh.colors[i++] = 0.15f;
        mesh.colors[i++] = 0.7f;
        mesh.colors[i++] = 0.26f;
        mesh.colors[i++] = 1.0f; // color alpha
    }

    for (int gz = 0; gz < VERTEX_COUNT - 1; gz++) {
        for (int gx = 0; gx < VERTEX_COUNT - 1; gx++) {
            int topLeft = (gz * VERTEX_COUNT) + gx;
            int topRight = topLeft +1;
            int bottomLeft = ((gz + 1) * VERTEX_COUNT) + gx;
            int bottomRight = bottomLeft + 1;

            mesh.indices[mesh.indicesCount] = topLeft;
            mesh.indicesCount++;
            mesh.indices[mesh.indicesCount] = bottomLeft;
            mesh.indicesCount++;
            mesh.indices[mesh.indicesCount] = topRight;
            mesh.indicesCount++;
            mesh.indices[mesh.indicesCount] = topRight;
            mesh.indicesCount++;
            mesh.indices[mesh.indicesCount] = bottomLeft;
            mesh.indicesCount++;
            mesh.indices[mesh.indicesCount] = bottomRight;
            mesh.indicesCount++;
        }
    }

    entity.meshes[0] = mesh;
    entity.meshCount = 1;

    return entity;
}

Buffer CreateBuffer(BufferRenderType type) {
  Buffer buffer;

  buffer.type = type;
  buffer.verticesBuffer = { 0 };
  buffer.colorsBuffer = { 0 };
  buffer.indexBuffer = { 0 };

  glGenVertexArrays(1, &buffer.vaoId);
  glGenBuffers(1, &buffer.verticesBuffer.bufferId);
  glGenBuffers(1, &buffer.colorsBuffer.bufferId);
  glGenBuffers(1, &buffer.indexBuffer.bufferId);

  // Allocate memory for Dynamic Buffers
  buffer.verticesBuffer.data = (float *)malloc(MAX_DYNAMIC_DATA_PER_BUFFER * sizeof(float));
  buffer.colorsBuffer.data = (float *)malloc(MAX_DYNAMIC_DATA_PER_BUFFER * sizeof(float));

  if (type == BufferRenderType::Elements) {
    buffer.indexBuffer.data = (int *)malloc(MAX_DYNAMIC_DATA_PER_BUFFER * sizeof(int));
  }

  return buffer;
}

void BindBuffer(int id) {
  bufferHandler.currentBuffer = id;
}


int GetCurrentBuffer() {
  return bufferHandler.currentBuffer;
}

void CleanBuffer(int id) {
  bufferHandler.buffers[id].verticesBuffer.vertexCount = 0;
  bufferHandler.buffers[id].colorsBuffer.vertexCount = 0;
  bufferHandler.buffers[id].indexBuffer.vertexCount = 0;
  bufferHandler.buffers[id].indexBuffer.triangleCount = 0;
}

void StoreBuffer(Buffer *buffer) {
  bufferHandler.buffers[bufferHandler.size] = *buffer;
  buffer->id = bufferHandler.size;
  bufferHandler.size += 1;
}

#endif // COD3R_GL_IMPLEMENTATION

