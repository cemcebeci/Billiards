#version 450
#extension GL_ARB_separate_shader_objects : enable

	float visible;
} ubo;

layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

	gl_Position = vec4(inPosition * ubo.visible, 0.5f, 1.0f);
	outUV = inUV;
}