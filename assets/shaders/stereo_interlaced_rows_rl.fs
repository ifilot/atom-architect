#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D left_eye_texture;
uniform sampler2D right_eye_texture;

uniform int screen_x;
uniform int screen_y;

layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;

void main()
{
    if (int(gl_FragCoord.y + screen_y) % 2 == 1) {
        FragColor = texture(left_eye_texture, TexCoords);
    } else {
        FragColor = texture(right_eye_texture, TexCoords);
    }
}
