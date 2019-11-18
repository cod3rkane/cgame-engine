#ifndef CGAME_ENGINE_INTERACTIONS_H
#define CGAME_ENGINE_INTERACTIONS_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "external/glad.h"
#include <GLFW/glfw3.h>
#if !defined(COD3R_GL_IMPLEMENTATION)
    #include "cod3rGL.h" // for Camera
#endif

void UserInputs(GLFWwindow *window, float deltaTime, Camera *camera);

#endif // CGAME_ENGINE_INTERACTIONS_H
