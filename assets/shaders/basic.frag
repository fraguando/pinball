#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uColor;
uniform vec3 uLightDir;   // direction TO the light, normalized
uniform vec3 uCameraPos;
uniform vec3 uEmissive;   // self-illumination added on top of lighting
uniform float uAlpha;     // 1 = opaque

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);

    float ambient = 0.25;
    float diffuse = max(dot(N, L), 0.0) * 0.75;

    // Simple Blinn-Phong specular highlight.
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0) * 0.3;

    vec3 color = uColor * (ambient + diffuse) + vec3(spec) + uEmissive;
    FragColor = vec4(color, uAlpha);
}
