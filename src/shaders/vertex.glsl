#version 410

in vec3 vertexPosition;
in vec4 vertexColor;

uniform mat4 mvp;             // VS: ModelViewProjection matrix
uniform mat4 projection;      // VS: Projection matrix
uniform mat4 view;            // VS: View matrix

out vec4 color;

void main() {
  gl_Position = mvp * vec4(vertexPosition, 1.0);
  color = vertexColor;
}
