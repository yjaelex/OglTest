#version 410 core

in block
{
    vec3 Color;
} Out;

// Ouput data
out vec3 color;

void main(){

	color = Out.Color;
}

