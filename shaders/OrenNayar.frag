#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2D tex;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    vec3 lightPos;
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
} gubo;

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, float sigma) {
    //vec3 V  - direction of the viewer
    //vec3 N  - normal vector to the surface
    //vec3 L  - light vector (from the light model)
    //vec3 Md - main color of the surface
    //float sigma - Roughness of the model
    
    float theta_i = acos(dot(L, N));
    float theta_r = acos(dot(V,N));
    float alpha = max(theta_i, theta_r);
    float beta = min(theta_i, theta_r);
    
    float sig_square = pow(sigma, 2);
    float A = 1 - 0.5 * (sig_square / (sig_square + 0.33));
    float B = 0.45 * (sig_square / (sig_square + 0.09));
    
    vec3 v_i = normalize(L - dot(L, N) * N);
    vec3 v_r = normalize(V - dot(V, N) * N);
    float G = max(0, dot(v_i,v_r));
    
    vec3 l = Md * max(0,dot(L, N));
    
    return l * (A + B * G * sin(alpha) * tan(beta));
}

const float beta = 2.0f;
const float g = 5.0f;
const float cosout = 0.7;
const float cosin  = 0.8;

void main() {
    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos - fragPos);
    
    float distance = length(gubo.lightPos - fragPos);
    vec3 lightDir = (gubo.lightPos - fragPos) / distance;
    float directionCosine = dot(lightDir, -gubo.lightDir);
    float dimmingTerm = (directionCosine - cosout) / (cosin - cosout);
    vec3 lightColor = gubo.lightColor.rgb * pow(g / distance, beta) * dimmingTerm;

    vec3 DiffSpec = BRDF(EyeDir, Norm, lightDir, texture(tex, fragUV).rgb, 1.1f);
    
    outColor = vec4(clamp(0.95 * (DiffSpec) * lightColor.rgb ,0.0,1.0), 1.0f);
}
