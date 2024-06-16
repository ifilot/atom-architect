#version 330 core

in vec2 uvs;

out vec4 fragColor;

void main() {
    // load color from texture
    vec3 color = vec3(1,1,1);
    float alpha = 1.0;
    fragColor = vec4(color, alpha);
}
