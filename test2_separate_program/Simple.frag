#version 410 core

// Ouput data
out vec4 color;

layout(location = 3) in vec3 InColor;

void main()
{
	color = vec4(InColor, 1.0);
}
