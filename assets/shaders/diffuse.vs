#version 330 core

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec2 uvs;
out vec4 normal2;

uniform mat4 mvp;

void main() {
    // output position of the vertex
    gl_Position = mvp * vec4(position, 1.0);

    normal2 = mvp * vec4(normal, 0.0);

    // output uv position
    uvs = uv;
}
