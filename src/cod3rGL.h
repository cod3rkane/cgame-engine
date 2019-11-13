#ifndef COD3R_GL_H
#define COD3R_GL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "external/glad.h"

#define LOG_WARNING "WARNING: "
#define LOG_DEBUG "DEBUG: "
#define LOG_INFO "INFO: "

#define DEFAULT_ATTRIB_POSITION_NAME "vertexPosition"
#define DEFAULT_ATTRIB_COLOR_NAME "vertexColor"
#define LOC_VERTEX_POSITION 0
#define LOC_VERTEX_COLOR 1

// Structs
typedef struct Shader {
    unsigned int id; // Shader Program ID
    int *locs;      // Shader locations array
} Shader;

typedef struct Mesh {
    int vertexCount;        // number of vertices stored in arrays
    int triangleCount;      // number of triangles stored (indexed or not)
    float *vertices;        // vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;       // vertex texture coordinates (UV - 2 components per vertex)
    float *texcoords2;      // vertex second texture coordinates (useful for lightmaps)
    float *normals;         // vertex normals (XYZ - 3 components per vertex)
    float *tangents;        // vertex tangents (XYZW - 4 components per vertex)
    unsigned char *colors;  // vertex colors (RGBA - 4 components per vertex)
    unsigned short *indices;// vertex indices (in case vertex data comes indexed)

    // Animation vertex data
    float *animVertices;    // Animated vertex positions (after bones transformations)
    float *animNormals;     // Animated normals (after bones transformations)
    int *boneIds;           // Vertex bone ids, up to 4 bones influence by vertex (skinning)
    float *boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning)

    // OpenGL identifiers
    unsigned int vaoId;     // OpenGL Vertex Array Object id
    unsigned int *vboId;    // OpenGL Vertex Buffer Objects id
} Mesh;

// Functions
Shader LoadShader(const char *vsFileName, const char *fsFileName);
Shader LoadShaderCode(const char *vsCode, const char *fsCode);
static unsigned int CompileShader(const char *shaderStr, int type);
static unsigned int LoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId);
void UnloadShader(Shader shader);
static void SetShaderDefaultLocations(Shader *shader);
char *LoadText(const char *fileName);

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
        textFile = fopen(fileName, "rt");

        if (textFile != NULL) {
            fseek(textFile, 0, SEEK_END);
            int size = ftell(textFile);
            fseek(textFile, 0, SEEK_SET);

            if (size > 0) {
                int count = fread(text, sizeof(char), size, textFile);
                text[count] = '\0';
            }

            fclose(textFile);
        } else {
            printf(LOG_WARNING, "[%s] Text file could not be opened\n", fileName);
        }
    }

    return text;
}

Shader LoadShaderCode(const char *vsCode, const char *fsCode) {
    Shader shader = { 0 };

    unsigned int vertexShaderId = 0;
    unsigned int fragmentShaderId = 0;

    if (vsCode != NULL) vertexShaderId = CompileShader(vsCode, GL_VERTEX_SHADER);
    if (fsCode != NULL) fragmentShaderId = CompileShader(fsCode, GL_FRAGMENT_SHADER);

    shader.id = LoadShaderProgram(vertexShaderId, fragmentShaderId);

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    if (shader.id == 0) printf(LOG_WARNING, "Custom shader could not be\n");

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
        printf(LOG_DEBUG, "[Shader ID: %i] Active uniform [%s] set at locatiom: %i\n", shader.id, name, location);
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
        printf(LOG_WARNING, "[Shader ID: %i] Failed to compile shader...\n", shader);
        int maxLength = 0;
        int length;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        char log[maxLength];
        glGetShaderInfoLog(shader, maxLength, &length, log);
        printf(LOG_INFO, "[Shader ID: %i] Shader compiled successfully\n", shader);
    }

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
        printf(LOG_WARNING, "[Shader ID: %i] Failed to link shader program...\n", program);
        int maxLength = 0;
        int length;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        char log[maxLength];

        glGetProgramInfoLog(program, maxLength, &length, log);

        printf(LOG_INFO, "%s", log);
    } else {
        printf(LOG_INFO, "[Shader ID: %i] Shader program loaded successfully\n", program);
    }

    return program;
}

void UnloadShader(Shader shader) {
    if (shader.id > 0) {
        glDeleteShader(shader.id);
        printf(LOG_INFO, "[Shader ID: %i] Unloaded shader program data\n", shader.id);
    }
}

static void SetShaderDefaultLocations(Shader *shader) {
    shader->locs[LOC_VERTEX_POSITION] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_POSITION_NAME);
    shader->locs[LOC_VERTEX_COLOR] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_COLOR_NAME);
}

#endif // COD3R_GL_H
