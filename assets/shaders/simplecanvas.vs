#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;
uniform float height;

void main()
{
    vec2 p = pos * 0.25 + vec2(0.75,-0.75);
    gl_Position = vec4(p, 0.0, 1.0);
    TexCoords = texCoords;
}
