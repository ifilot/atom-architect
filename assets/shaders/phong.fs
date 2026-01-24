#version 330 core

in vec3 vertex_direction_eyespace;
in vec3 lightdirection_eyespace;
in vec3 normal_eyespace;

uniform vec3 color;

out vec4 fragColor;

// Tunable parameters
const float ambient_strength  = 0.05;
const float specular_strength = 0.4;
const float shininess         = 64.0;   // higher = tighter highlight

void main()
{
    // Normalize inputs
    vec3 N = normalize(normal_eyespace);
    vec3 L = normalize(lightdirection_eyespace);
    vec3 V = normalize(vertex_direction_eyespace);

    // --- Ambient ---
    vec3 ambient = ambient_strength * color;

    // --- Diffuse (Lambert) ---
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * color;

    // --- Specular (Blinn-Phong, more stable than reflect()) ---
    vec3 H = normalize(L + V);   // half-vector
    float NdotH = max(dot(N, H), 0.0);
    float spec  = pow(NdotH, shininess);

    vec3 specular = specular_strength * spec * vec3(1.0);

    // --- Optional rim lighting (helps thin bonds & silhouettes) ---
    float rim = pow(1.0 - max(dot(N, V), 0.0), 2.0);
    vec3 rim_light = 0.15 * rim * color;

    // --- Combine ---
    vec3 result = ambient + diffuse + specular + rim_light;

    // --- Gamma correction (CRITICAL for correct appearance) ---
    result = pow(result, vec3(1.0 / 2.2));

    fragColor = vec4(result, 1.0);
}
