#version 410 core

// tessellation primitive generator consumes out_patch and out u,v.
// per tcs out_patch -- TEG --> (u ,v, w);
// u,v is just like barycentric coordinates;(For triangles, it's barycentric u+v+w=1)
// the TEG doesn't even have access to the patch.
// Its only output are barycentric coordinates and their connectivity.


// the tessellation evaluation shader takes the (u; v) or (u; v; w) subdivided by
// the TEG, and generates a vertex with a position and associated attributes.
// TES has access both to the out_patch of the TCS and the (u,v) of TEG.
// The TEG executes the TES on every barycentric coordinate(u,v) and
// the job of the TES is to generate a vertex for that point(u,v).
// out_ptach & (u,v) -- TES --> gl_Position


// For example, for one in_patch with 3 vertex:
// in_patch{ pos[3] } --> TCS --> out_patch{ pos[4] } & TessLevel{In/Out};
// (TCS run 4 times; in patch is 3 vertex; out patch is 4 vertex)
// per_out_patch & TessLevel --> TEG --> (u,v)[N]; (TEG will out lots of (u,v) points, depends on TessLevel)
// per_(u,v)_point --> TES --> 

// must specify primitive mode(triangles, quads, or isolines),
// which specify that the TEG should subdivide a triangle into smaller triangles,
// a quad into triangles, or a quad into a collection of lines,
layout(triangles, equal_spacing, ccw) in;

in block
{
    vec3 Color;
    vec4 Center;
} In[];

out block
{
    vec3 Color;
    vec4 Center;
} Out;


// out_patch of TCS
in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

out gl_PerVertex 
{
    vec4 gl_Position;
};

uniform CB3
{
	float tesScale;
} cb3;

vec4 interpolate4D(vec4 v0, vec4 v1, vec4 v2)
{
    return vec4(gl_TessCoord.x) * v0 + vec4(gl_TessCoord.y) * v1 + vec4(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{	
    gl_Position = interpolate4D(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);

    Out.Center = In[0].Center;

    vec3 color = interpolate3D(In[2].Color, In[0].Color, In[1].Color);
    if(gl_TessCoord.x < 0.5)
    {
        Out.Color = cb3.tesScale * color;
    }
    else
    {
        Out.Color = 1 - color;
    }
}

