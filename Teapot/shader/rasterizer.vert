#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 ipos;
layout (location = 1) in vec3 icolor;

layout (location = 0) out vec3 fragColor;

layout (binding = 0) uniform TransformBinding
{
	mat4 model;
	mat4 view;
	mat4 proj;
} tr;

void main()
{
    gl_Position = tr.proj * tr.view * tr.model * vec4(ipos, 1.0);
    fragColor = icolor;
}