#version 410 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;

out block
{
    vec3 Color;
} Out; 

out gl_PerVertex
{
    vec4 gl_Position;
};

uniform mat4 MVP;

uniform CB0
{
	vec3 diffuseColor;
} cb0;

void main()
{
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);

	// The color of each vertex will be interpolated
	// to produce the color of each fragment
	Out.Color = vertexColor * cb0.diffuseColor;
}

