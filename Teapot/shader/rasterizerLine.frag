#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 selected;

layout(location = 0) out vec4 outColor;

void main()
{
    if (selected.y == 1)
    {
        outColor = vec4(1.0, 0.55, 0.0, 1.0);
    }
    else
    {
        outColor = vec4(0.25, 0.25, 0.25, 1.0);
    }
}