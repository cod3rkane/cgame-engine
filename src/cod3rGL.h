#ifndef COD3R_GL_H
#define COD3R_GL_H

#include <stdio.h>
#include <stdlib.h>
#include "external/glad.h"
#include "utils.h"
#include <cglm/cglm.h>

#define DEFAULT_ATTRIB_POSITION_NAME "vertexPosition"
#define DEFAULT_ATTRIB_COLOR_NAME "vertexColor"
#define MAX_SHADER_LOCATIONS 32      // Maximum number of predefined locations stored in shader struct
#define MAX_DYNAMIC_DATA_PER_BUFFER 50000 // Maximum number of items per Dynamic Buffer

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
    mat4 matrix; // Local transform matrix

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

// Global Variables

unsigned int currentVaoId = 0;

DynamicFBuffer currentVerticesBuffer = { 0 };
DynamicFBuffer currentColorsBuffer = { 0 };
DynamicIBuffer currentIndexBuffer = { 0 };

Shader defaultShader;

mat4 view = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};
mat4 projection = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};
mat4 model = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

// Functions
Shader LoadShader(const char *vsFileName, const char *fsFileName);
Shader LoadShaderCode(const char *vsCode, const char *fsCode);
static unsigned int CompileShader(const char *shaderStr, int type);
static unsigned int LoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId);
void UnloadShader(Shader shader);
static void SetShaderDefaultLocations(Shader *shader);
char *LoadText(const char *fileName);

Entity CreateRect(Vector4 *color, vec3 position);
void DrawRect(Mesh mesh);

void DrawEntity(Entity entity);
void RotateEntity(Entity *entity, float angle);

void InitCod3rGL(int windowWidth, int windowHeight); // Initialise all global variables and other setups.
void CleanCod3rGL();
void RenderCod3rGL();

void StoreDataToBufferf(DynamicFBuffer *buffer, float *data, int dataSize);
void StoreDataToBufferi(DynamicIBuffer *buffer, int *data, int dataSize, int numTriangles);
void CleanCurrentBuffers();

// Functions Declarations

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
            TraceLog(LOG_WARNING, "[%s] Text file could not be opened\n", fileName);
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

    if (shader.id == 0) TraceLog(LOG_WARNING, "Custom shader could not be\n");

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
        TraceLog(LOG_DEBUG, "[Shader ID: %i] Active uniform [%s] set at locatiom: %i\n", shader.id, name, location);
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
        TraceLog(LOG_WARNING, "[Shader ID: %i] Failed to compile shader...\n", shader);
        int maxLength = 0;
        int length;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        char log[maxLength];
        glGetShaderInfoLog(shader, maxLength, &length, log);
        TraceLog(LOG_INFO, "%s", log);
    }

    TraceLog(LOG_INFO, "[Shader ID: %i] Shader compiled successfully\n", shader);

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
        TraceLog(LOG_WARNING, "[Program ID: %i] Failed to link shader program...\n", program);
        int maxLength = 0;
        int length;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        char log[maxLength];

        glGetProgramInfoLog(program, maxLength, &length, log);

        TraceLog(LOG_INFO, "%s", log);
    } else {
        TraceLog(LOG_INFO, "[Program ID: %i] Shader program loaded successfully\n", program);
    }

    return program;
}

void UnloadShader(Shader shader) {
    if (shader.id > 0) {
        glDeleteShader(shader.id);
        TraceLog(LOG_INFO, "[Program ID: %i] Unloaded shader program data\n", shader.id);
    }
}

static void SetShaderDefaultLocations(Shader *shader) {
    shader->locs[LOC_VERTEX_POSITION] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_POSITION_NAME);
    shader->locs[LOC_VERTEX_COLOR] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_COLOR_NAME);

    shader->locs[LOC_MATRIX_PROJECTION] = glGetUniformLocation(shader->id, "projection");
    shader->locs[LOC_MATRIX_VIEW] = glGetUniformLocation(shader->id, "view");
    shader->locs[LOC_MATRIX_MODEL] = glGetUniformLocation(shader->id, "model");
}

Entity CreateRect(Vector4 *color, vec3 position) {
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
        color = malloc(sizeof(Vector2));
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

    mat4 matrix = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    glm_translate(matrix, position);
    glm_mat4_copy(matrix, entity.matrix);

    entity.meshes[0] = mesh;
    entity.meshCount = 1;

    return entity;
}

void DrawRect(Mesh mesh) {
    // @TODO: transformations
    StoreDataToBufferf(&currentVerticesBuffer, mesh.vertices, 12);
    StoreDataToBufferf(&currentColorsBuffer, mesh.colors, 16);
    StoreDataToBufferi(&currentIndexBuffer, mesh.indices, 6, 4);
}

void RenderCod3rGL() {
    // @TODO: 3D render
    // @TODO: 2D render
    glUseProgram(defaultShader.id);
    glBindVertexArray(currentVaoId);

    glBindBuffer(GL_ARRAY_BUFFER, currentVerticesBuffer.bufferId);
    glBufferData(GL_ARRAY_BUFFER, currentVerticesBuffer.vertexCount * sizeof(float), currentVerticesBuffer.data, GL_STATIC_DRAW);

    glVertexAttribPointer(LOC_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(LOC_VERTEX_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, currentColorsBuffer.bufferId);
    glBufferData(GL_ARRAY_BUFFER, currentColorsBuffer.vertexCount * sizeof(float), currentColorsBuffer.data, GL_STATIC_DRAW);

    glVertexAttribPointer(LOC_VERTEX_COLOR, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(LOC_VERTEX_COLOR);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentIndexBuffer.bufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, currentIndexBuffer.vertexCount * sizeof(unsigned int), currentIndexBuffer.data, GL_STATIC_DRAW);

    if (defaultShader.locs[LOC_MATRIX_PROJECTION] != -1) {
        glUniformMatrix4fv(defaultShader.locs[LOC_MATRIX_PROJECTION], 1, GL_FALSE, projection[0]);
    }

    if (defaultShader.locs[LOC_MATRIX_VIEW] != -1) {
        glUniformMatrix4fv(defaultShader.locs[LOC_MATRIX_VIEW], 1, GL_FALSE, view[0]);
    }

    if (defaultShader.locs[LOC_MATRIX_MODEL] != -1) {
        glUniformMatrix4fv(defaultShader.locs[LOC_MATRIX_MODEL], 1, GL_FALSE, model[0]);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentIndexBuffer.bufferId);

    glDrawElements(GL_TRIANGLES, currentIndexBuffer.vertexCount, GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);

    glBindVertexArray(0);
    glUseProgram(0);

    CleanCurrentBuffers(); // Reset all currentbuffers
}

void InitCod3rGL(int windowWidth, int windowHeight) {
    // Initialise buffers
    glGenVertexArrays(1, &currentVaoId);
    glGenBuffers(1, &currentVerticesBuffer.bufferId);
    glGenBuffers(1, &currentColorsBuffer.bufferId);
    glGenBuffers(1, &currentIndexBuffer.bufferId);

    // Allocate memory for Dynamic Buffers
    currentVerticesBuffer.data = (float *)malloc(MAX_DYNAMIC_DATA_PER_BUFFER * sizeof(float));
    currentColorsBuffer.data = (float *)malloc(MAX_DYNAMIC_DATA_PER_BUFFER * sizeof(float));
    currentIndexBuffer.data = (int *)malloc(MAX_DYNAMIC_DATA_PER_BUFFER * sizeof(int));

    defaultShader = LoadShader("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");

    // setup matrices
    glm_perspective(glm_rad(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f, projection);
    glm_translate(view, (vec3){0.0f, 0.0f, -3.0f});
}

void CleanCod3rGL() {
    glDeleteVertexArrays(1, &currentVaoId);
    glDeleteBuffers(1, &currentVerticesBuffer.bufferId);
    glDeleteBuffers(1, &currentColorsBuffer.bufferId);
    glDeleteBuffers(1, &currentIndexBuffer.bufferId);
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

void CleanCurrentBuffers() {
    currentVerticesBuffer.vertexCount = 0;
    currentColorsBuffer.vertexCount = 0;
    currentIndexBuffer.vertexCount = 0;
    currentIndexBuffer.triangleCount = 0;
}

void DrawEntity(Entity entity) {
    for (int i = 0; i < entity.meshCount; i++) {
        // apply matrix to vertex
        float formattedVertex[entity.meshes[i].vertexCount];
        vec4 newPos = {0.0f, 0.0f, 0.0f, 0.0f};
        for (int vertIndex = 0; vertIndex < entity.meshes[i].vertexCount;) {
            glm_mat4_mulv(entity.matrix, (vec4){entity.meshes[i].vertices[vertIndex], entity.meshes[i].vertices[vertIndex + 1], entity.meshes[i].vertices[vertIndex + 2], 0.01f}, newPos);
            formattedVertex[vertIndex] = newPos[0];
            formattedVertex[vertIndex + 1] = newPos[1];
            formattedVertex[vertIndex + 2] = newPos[2];

            vertIndex++;
            vertIndex++;
            vertIndex++;
        }

        StoreDataToBufferf(&currentVerticesBuffer, formattedVertex, entity.meshes[i].vertexCount);
        StoreDataToBufferf(&currentColorsBuffer, entity.meshes[i].colors, entity.meshes[i].vertexCount + entity.meshes[i].triangleCount);
        StoreDataToBufferi(&currentIndexBuffer, entity.meshes[i].indices, entity.meshes[i].indicesCount, entity.meshes[i].triangleCount);
    }
}

void RotateEntity(Entity *entity, float angle) {
    mat4 matrix = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    glm_rotate(entity->matrix, angle, (vec3){ 0.0f, 0.0f, 1.0f });
}

#endif // COD3R_GL_H
