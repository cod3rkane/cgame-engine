#version 410

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec4 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 color;

void main() {
  gl_Position = projection * view * vec4(vertexPosition, 1.0);
  color = vertexColor;
}
