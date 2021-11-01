#version 410 core

in block
{
    vec3 Color;
} Out;

uniform CB0
{
	float scale[];
} cb0;

// Ouput data
out vec3 color;

void main(){

	color = Out.Color + cb0.scale[0];
}

