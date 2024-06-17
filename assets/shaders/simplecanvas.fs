#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D regular_texture;

layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;

void main()
{
    vec3 col = texture(regular_texture, TexCoords).rgb;
    if(length(col) > .1) {
        FragColor = vec4(col, 1.0f);
    } else {
        FragColor = vec4(col, 0.0f);
    }
}
