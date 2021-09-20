#version 410 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

out block
{
    vec3 VertColor;
} Out; 

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(){

    gl_Position.xyz = vertexPosition_modelspace;
    gl_Position.w = 1.0;

    vec3 offset = mix(vertexPosition_modelspace, vec3(1.0, 1.0, 0.0), 0.3);
    offset = offset / 2.0;
    Out.VertColor = vec3(0.2, 0.1, 0.2) + offset;
}
