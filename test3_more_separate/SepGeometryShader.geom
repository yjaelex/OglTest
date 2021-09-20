#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 4) out;

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

in block
{
    vec3 VertColor;
} In[]; 

out gl_PerVertex 
{
    vec4 gl_Position;
};

out vec3 GSColor;

void main()
{
    for(int i = 0; i < gl_in.length(); ++i)
    {
        gl_Position = gl_in[i].gl_Position;
        GSColor = In[i].VertColor;
        EmitVertex();
    }

    gl_Position = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3;
    gl_Position.w = 1.0;
    GSColor = vec3(1.0, 1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}

