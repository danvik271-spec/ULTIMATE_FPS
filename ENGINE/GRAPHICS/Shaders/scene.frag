#version 450
layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 outColor;
void main() {
    vec2 centered = vUv - vec2(0.5);
    float vignette = 1.0 - smoothstep(0.18, 0.95, length(centered) * 1.45);
    vec3 upper = vec3(0.035, 0.105, 0.17);
    vec3 lower = vec3(0.012, 0.025, 0.052);
    vec3 color = mix(lower, upper, clamp(vUv.y, 0.0, 1.0));
    color *= vignette;
    outColor = vec4(color, 1.0);
}
