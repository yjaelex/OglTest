#version 410 core

// Ouput data
out vec4 color;

in block
{
	vec3 TesColor;
} Out;

void main()
{
	color = vec4(Out.TesColor, 1.0);
}
