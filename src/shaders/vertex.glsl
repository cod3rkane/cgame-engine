#version 410

layout (location = 0) in vec3 vertexPosition;

out vec4 color;

void main() {
  gl_Position = vec4(vertexPosition, 1.0);
}
