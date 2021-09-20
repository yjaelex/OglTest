#version 410 core

// Ouput data
out vec4 color;

in block
{
    vec3 GSColor;
} Out;

void main()
{
    color = vec4(Out.GSColor, 1.0);
}
