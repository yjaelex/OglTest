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

uniform mat4 MV;

void main(){	

	gl_Position =  MV * vec4(vertexPosition_modelspace,1);

	Out.Color = vertexColor;
}

