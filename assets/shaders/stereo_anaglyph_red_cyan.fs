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
    vec3 coeff = vec3(.2126, .7152, .0722);  // CIE 1931 linear luminance
    float gray_left = dot(texture(left_eye_texture, TexCoords).rgb, coeff);
    float gray_right = dot(texture(right_eye_texture, TexCoords).rgb, coeff);
    FragColor = vec4(1.0, 0.0, 0.0, 0.0) * gray_left +
        vec4(0.0, 1.0, 1.0, 0.0) * gray_right +
        vec4(0.0, 0.0, 0.0, 1.0);
}
