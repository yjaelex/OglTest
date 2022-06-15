
// Include GLEW
#include <GL/glew.h>

#include <common/shader.hpp>
#include <common/main.h>

#ifdef _WIN32
#include <common/_getopt.h>
#else
#ifdef LINUX
#include <getopt.h>
#endif
#endif

GLuint vertexbuffer;
GLuint VertexArrayID;
GLuint programID;
GLboolean bBeginEnd = 1;

const char* gFSStr = "#version 330\n"
                    "// Ouput data\n"
                    "in vec4 incolor;\n"
                    "out vec4 color;\n"
                    "void main()\n"
                    "{\n"
                    "    color = incolor;\n"
                    "}\n";

const char* gVSStr = "#version 330\n"
                    "// Input vertex data, different for all executions of this shader.\n"
                    //"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
                    "out vec4 incolor;\n"
                    "void main() {\n"
                    "    gl_Position.xyz = gl_Vertex.xyz;\n"
                    "    gl_Position.w = 1.0;\n"
                    "    incolor = gl_Color;\n"
                    "}\n";

void ProcessCommandLine(int argc, char* argv[])
{
    static struct option long_options[] =
    {
        /* These options don¡¯t set a flag.*/
        {"vbo",     no_argument, 0, 'v'},
        {"help",    no_argument, 0, 'h'},

        {0, 0, 0, 0}
    };
    int c = 0;

    while (1)
    {
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long(argc, argv, "hv", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            log("option %s", long_options[option_index].name);
            if (optarg)
                log(" with arg %s", optarg);
            log("\n");
            break;

        case 'v':
            bBeginEnd = 0;
            break;

        case 'h':
            error("Options:\n"
                "  --vbo, -v : VBO mode, instead of glBegin/glEnd.\n"
                "  --help, -h    : Print this help.\n");
            break;

        case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            error("Internal errors when calling getopt_long()");
        }
    }
}

bool InitGL(size_t Width, size_t Height)
{
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        error("Failed to initialize GLEW\n");
        return false;
    }

    const char* versionGL = "\0";
    versionGL = (char*)(glGetString(GL_VERSION));
    log("OpenGL version: %s\n", versionGL);
    log("GLEW version: %d.%d.%d.%d\n",  GLEW_VERSION, GLEW_VERSION_MAJOR,
                                        GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    //programID = LoadShaders( "SimpleVertexShader.vert", "SimpleFragmentShader.frag" );

    GLenum type[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
    std::string str[2];
    str[0].assign(gVSStr);
    str[1].assign(gFSStr);
    programID = CreateProgramFromStrings(type, str, 2);

    static const GLfloat g_vertex_buffer_data[] = { 
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,
    };

    log("[DRAW_MODE] - %s \n", bBeginEnd ? "Use glBegin&glEnd mode!" : "Use VBO mode!");

    if (!bBeginEnd)
    {
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    }

    return true;
}

void ReSizeGLScene(size_t Width, size_t Height)
{
    glViewport(0, 0, GLsizei(Width), GLsizei(Height));
}

void DrawGLScene(void)
{
    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT );

    // Use our shader
    glUseProgram(programID);

    if (bBeginEnd)
    {
        glColor4f(0.5, 0.5, 0.1, 1.0);
        glBegin(GL_TRIANGLES);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glEnd();
#define WW 8
#define HH 4
        glBegin(GL_TRIANGLE_STRIP);
            GLfloat c0[] = {0.9843137, 0.9843137, 0.09882353, 1};
            glColor4fv(c0);
            GLfloat v0[] = {-6.076823 / WW, 3.486979 / HH, 0};
            glVertex3fv(v0);
            GLfloat c1[] = { 0.9333333, 0.09411765, 0.945098, 1 };
            glColor4fv(c1);
            GLfloat v1[] = { -6.076823 / WW, -3.486979 / HH, 0 };
            glVertex3fv(v1);
            GLfloat c2[] = { 0.09843137, 0.9843137, 0.9882353, 1 };
            glColor4fv(c2);
            GLfloat v2[] = { 6.076823 / WW, 3.486979 / HH, 0 };
            glVertex3fv(v2);
            GLfloat c3[] = { 0.9333333, 0.09411765, 0.945098, 1 };
            glColor4fv(c3);
            GLfloat v3[] = { 6.076823 / WW, -3.486979 / HH, 0 };
            glVertex3fv(v3);
        glEnd();
    }
    else
    {
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

        glDisableVertexAttribArray(0);
    }
}

void DeInitGL(void)
{
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);
    return;
}
