#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 4) out;

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

in block
{
    vec3 Color;
} In[];

out gl_PerVertex 
{
    vec4 gl_Position;
};

out block
{
    vec3 Color;
} Out;

uniform mat4 P;

void main()
{
    for(int i = 0; i < gl_in.length(); ++i)
    {
        gl_Position = P * gl_in[i].gl_Position;
        Out.Color = In[i].Color;
        EmitVertex();
    }

    vec4 avgPos = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3;
    gl_Position = P * (vec4(0.0, 0.0f, 0.1, 0.0) + avgPos);

    Out.Color = vec3(1.0, 1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}

