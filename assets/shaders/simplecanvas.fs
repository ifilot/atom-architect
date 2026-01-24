#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D regular_texture;

void main()
{
    vec4 c = texture(regular_texture, TexCoords);
    FragColor = c;
}
