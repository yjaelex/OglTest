#version 410 core

// The tessellation control shader consumes an input patch provided by the application
// and emits a new output patch.
// patch --> tcs --> out_patch;
// Each invocation of tcs gen one vertex of out_patch.

// tessellation primitive generator consumes the input patch (out_patch of tcs)
// and produces a new set of basic primitives (points, lines, or triangles).

// must specify out patches vertex count
layout(vertices = 3) out;

in block
{
    vec3 Color;
    vec4 Center;
} In[];

out block
{
    vec3 Color;
    vec4 Center;
} Out[];

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

out gl_PerVertex 
{
    vec4 gl_Position;
} gl_out[];

uniform vec2 tessLevel;

void main()
{	
    gl_TessLevelInner[0] = tessLevel.x;
    gl_TessLevelInner[1] = tessLevel.x;
    gl_TessLevelOuter[0] = tessLevel.y;
    gl_TessLevelOuter[1] = tessLevel.y;
    gl_TessLevelOuter[2] = tessLevel.y;
    gl_TessLevelOuter[3] = tessLevel.y;
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    Out[gl_InvocationID].Color = In[gl_InvocationID].Color;
    Out[gl_InvocationID].Center = In[gl_InvocationID].Center;
}


