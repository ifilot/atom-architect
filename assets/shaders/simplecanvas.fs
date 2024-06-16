#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D regular_texture;

layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;

void main()
{
    vec3 canvas = texture(regular_texture, TexCoords).rgb;
    FragColor = vec4(canvas, 1.0f);
}
