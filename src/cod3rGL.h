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
    float *vertices;            // vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;           // vertex texture coordinates (UV - 2 components per vertex)
    float *texcoords2;          // vertex second texture coordinates (useful for lightmaps)
    float *normals;             // vertex normals (XYZ - 3 components per vertex)
    float *tangents;            // vertex tangents (XYZW - 4 components per vertex)
    float *colors;              // vertex colors (RGBA - 4 components per vertex)
    unsigned int *indices;    // vertex indices (in case vertex data comes indexed)

    // Animation vertex data
    float *animVertices;    // Animated vertex positions (after bones transformations)
    float *animNormals;     // Animated normals (after bones transformations)
    int *boneIds;           // Vertex bone ids, up to 4 bones influence by vertex (skinning)
    float *boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning)

    // OpenGL identifiers
    unsigned int vaoId;     // OpenGL Vertex Array Object id
    unsigned int *vboId;    // OpenGL Vertex Buffer Objects id
} Mesh;

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
    int length;
    unsigned int bufferId;
    int *data;
} DynamicIBuffer;

// Global Variables

static int drawCalls = 0;
static int currentBuffer = 0;
unsigned int currentVaoId = 0;

DynamicIBuffer currentElementBuffer;

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

Mesh createRect(Vector4 *color, vec3 position);
void drawRect(Mesh mesh);

void render();

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

Mesh createRect(Vector4 *color, vec3 position) {
    Mesh mesh = { 0 };
    mesh.vertices = (float *)malloc( 12 * sizeof(float));
    mesh.colors = (float *)malloc( 16 * sizeof(float));
    mesh.indices = (unsigned int *)malloc(6 * sizeof(unsigned int));
    mesh.vboId = (unsigned int *)malloc(4 * sizeof(unsigned int *));

    mesh.vboId[0] = 0; // positions
    mesh.vboId[1] = 0; // colors
    mesh.vboId[2] = 0; // indices

    glGenBuffers(1, &mesh.vboId[0]);
    glGenBuffers(1, &mesh.vboId[1]);
    glGenBuffers(1, &mesh.vboId[2]);
    glGenBuffers(1, &mesh.vboId[3]);

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
    vec4 newPos = { 0.0f, 0.0f, 0.0f, 0.0f };

    for (int i = 0; i < 12;) {
        glm_mat4_mulv(matrix, (vec4){ mesh.vertices[i], mesh.vertices[i + 1], mesh.vertices[i + 2], 1.0f }, newPos);
        mesh.vertices[i] = newPos[0];
        mesh.vertices[i + 1] = newPos[1];
        mesh.vertices[i + 2] = newPos[2];

        i++;
        i++;
        i++;
    }

    return mesh;
}

void drawRect(Mesh mesh) {
    glBindVertexArray(currentVaoId); // current global VAO

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), mesh.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(LOC_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(LOC_VERTEX_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[1]);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), mesh.colors, GL_STATIC_DRAW);

    glVertexAttribPointer(LOC_VERTEX_COLOR, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(LOC_VERTEX_COLOR);

    // GLint size = 0;
    // glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentElementBuffer.bufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * sizeof(unsigned int), mesh.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void render() {
    // @TODO: 3D render
    // @TODO: 2D render
    // @TODO: create dynamic buffer
    glUseProgram(defaultShader.id); // @TODO: create initializer
    glBindVertexArray(currentVaoId);

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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentElementBuffer.bufferId);

    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    // glDrawArrays(GL_TRIANGLES, 0, 12);

    glDisable(GL_BLEND);

    glBindVertexArray(0);
}

#endif // COD3R_GL_H
