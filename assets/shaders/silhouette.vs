#version 330 core

in vec3 position;
in vec3 normal;

uniform mat4 mvp;

void main() {
    // output position of the vertex
    gl_Position = mvp * vec4(position, 1.0);
}
