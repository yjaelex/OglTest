#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 5) out;

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

in block
{
    vec3 Color;
    vec4 Center;
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
uniform float normScale;

void main()
{
    gl_Position = P * gl_in[0].gl_Position;
    Out.Color = In[0].Color;
    EmitVertex();

    gl_Position = P * gl_in[1].gl_Position;
    Out.Color = In[1].Color;
    EmitVertex();

    vec4 avgPos = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3;
    vec3 normDir = (avgPos - In[0].Center).xyz;
    normDir = normalize(normDir);
    gl_Position = P * (vec4(normDir * normScale, 0.0) + avgPos);
    Out.Color = vec3(1.0, 1.0, 1.0);
    EmitVertex();

    gl_Position = P * gl_in[2].gl_Position;
    Out.Color = In[2].Color;
    EmitVertex();

    gl_Position = P * gl_in[0].gl_Position;
    Out.Color = In[0].Color;
    EmitVertex();

    EndPrimitive();
}

