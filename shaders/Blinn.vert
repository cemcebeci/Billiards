#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
    mat4 wMat;
    mat4 nMat;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNorm;
layout(location = 2) out vec2 outUV;

void main() {
	gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);
	outUV = inUV;
    outPosition = (ubo.wMat * vec4(inPosition, 1.0)).xyz;
    outNorm = (ubo.nMat * vec4(inNorm, 1.0)).xyz;
}
