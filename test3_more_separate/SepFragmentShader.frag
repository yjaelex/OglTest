#version 410 core

// Ouput data
out vec4 color;

in vec3 GSColor;

void main()
{
    color = vec4(GSColor, 1.0);
}
