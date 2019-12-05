#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 ipos;
layout (location = 1) in vec3 icolor;

layout (location = 0) out vec3 fragColor;

layout (binding = 0) uniform CameraBinding
{
	mat4 view;
	mat4 proj;
} camera;
layout (binding = 1) uniform ObjectBinding
{
	mat4 model;
} object;

void main()
{
    gl_Position = camera.proj * camera.view * object.model * vec4(ipos, 1.0);
    fragColor = icolor;

	gl_PointSize = 10;
}