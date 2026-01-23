#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D regular_texture;
uniform sampler2D silhouette_texture;
uniform int outline_radius;

vec4 calculate_border() {
    vec3 pixcol = texture(silhouette_texture, TexCoords).rgb;

    if (pixcol != vec3(0.0f)) {
        vec2 size = 2.0f / textureSize(silhouette_texture, 0);

        const int sz = 1;
        for (int i = -sz; i <= sz; i++) {
            for (int j = -sz; j <= sz; j++) {
                if (i == 0 && j == 0) {
                    continue;
                }

                vec2 offset = vec2(i, j) * size;

                if (texture(silhouette_texture, TexCoords + offset).rgb != pixcol) {

                    if(pixcol.b > 0.4f) {
                        // secondary buffer color
                        return vec4(236.f/255.f, 115.f/255.f, 255.f/255.f, 0.8f);
                    } else {
                        // primary buffer color
                        return vec4(67.f/255.f, 247.f/255.f, 181.f/255.f, 0.8f);
                    }
                }
            }
        }
    }

    return vec4(vec3(0.0f), 0.0f);
}

void main()
{
    vec3 canvas = texture(regular_texture, TexCoords).rgb;
    vec4 border = calculate_border();

    FragColor.rgb = mix(canvas, border.rgb, border.a);
    FragColor.a   = 1.0;
}
