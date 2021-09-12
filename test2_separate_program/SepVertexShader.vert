#version 410 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

layout(location = 3) out vec3 VertColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(){

    gl_Position.xyz = vertexPosition_modelspace;
    gl_Position.w = 1.0;

    vec3 offset = vertexPosition_modelspace + vec3(1.0, 1.0, 0.0);
    offset = offset / 4.0;
    VertColor = vec3(0.2, 0.1, 0.2) + offset;
}

