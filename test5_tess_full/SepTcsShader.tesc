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
    vec3 VertColor;
} In[];

out block
{
    vec3 TcsColor;
} Out[];

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

out gl_PerVertex 
{
    vec4 gl_Position;
} gl_out[];

void main()
{	
    gl_TessLevelInner[0] = 16.0;
    gl_TessLevelInner[1] = 16.0;
    gl_TessLevelOuter[0] = 8.0;
    gl_TessLevelOuter[1] = 8.0;
    gl_TessLevelOuter[2] = 8.0;
    gl_TessLevelOuter[3] = 8.0;
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    Out[gl_InvocationID].TcsColor = In[gl_InvocationID].VertColor;
}


