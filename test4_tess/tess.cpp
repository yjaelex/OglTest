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
GLuint PipelineName;

static int enWireFrame = 0;
static int verboseFlag = 0;
static int useSep = 0;

void ProcessCommandLine(int argc, char* argv[])
{
    static struct option long_options[] =
    {
        /* These options set a flag. */
        // "--verbose" long option just set verboseFlag=1
        {"verbose", no_argument, &verboseFlag, 1},
        {"lines",   no_argument, &enWireFrame, 1},

        /* These options don’t set a flag.*/
        // "--sep" long option; same with "-s"
        {"sep",     no_argument, 0, 's'},
        {"help",    no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int c = 0;

    while (1)
    {
        /* getopt_long stores the option index here. */
        int option_index = 0;
          c = getopt_long(argc, argv, "hsv", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

          switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
              log ("option %s", long_options[option_index].name);
              if (optarg)
                log (" with arg %s", optarg);
              log ("\n");
              break;

        case 's':
              useSep = 1;
              break;
        case 'v':
              verboseFlag = 1;
              break;

        case 'h':
            error("Options:\n  --sep, -s     : Enable separate shader objects.\n"
                             "  --lines       : Enable WireFrame mode.\n"
                             "  --verbose, -v : Verbose mode.\n"
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

GLuint SeparateProgramName[MAX];

bool InitSeparateProgram()
{
    bool Validated = true;

    log("Init Separate Program!\n");

    if (Validated)
    {
        std::string VertexSourceContent = FileContentsToString("SepVertexShader.vert");
        char const* VertexSourcePointer = VertexSourceContent.c_str();
        SeparateProgramName[VERTEX] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &VertexSourcePointer);
    }

    if (Validated)
    {
        std::string TcsSourceContent = FileContentsToString("SepTcsShader.tesc");
        char const* TcsSourcePointer = TcsSourceContent.c_str();
        SeparateProgramName[TESS_CONTROL] = glCreateShaderProgramv(GL_TESS_CONTROL_SHADER, 1, &TcsSourcePointer);
    }

    if (Validated)
    {
        std::string TesSourceContent = FileContentsToString("SepTesShader.tese");
        char const* TesSourcePointer = TesSourceContent.c_str();
        SeparateProgramName[TESS_EVALUATION] = glCreateShaderProgramv(GL_TESS_EVALUATION_SHADER, 1, &TesSourcePointer);
    }

    if (Validated)
    {
        std::string FragmentSourceContent = FileContentsToString("SepFragmentShader.frag");
        char const* FragmentSourcePointer = FragmentSourceContent.c_str();
        SeparateProgramName[FRAGMENT] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FragmentSourcePointer);
    }

    if (Validated)
    {
        Validated = Validated && CheckProgram(SeparateProgramName[VERTEX]);
        Validated = Validated && CheckProgram(SeparateProgramName[TESS_CONTROL]);
        Validated = Validated && CheckProgram(SeparateProgramName[TESS_EVALUATION]);
        Validated = Validated && CheckProgram(SeparateProgramName[FRAGMENT]);
    }

    if (Validated)
    {
        glGenProgramPipelines(1, &PipelineName);
        glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, SeparateProgramName[VERTEX]);
        glUseProgramStages(PipelineName, GL_TESS_CONTROL_SHADER_BIT, SeparateProgramName[TESS_CONTROL]);
        glUseProgramStages(PipelineName, GL_TESS_EVALUATION_SHADER_BIT, SeparateProgramName[TESS_EVALUATION]);
        glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, SeparateProgramName[FRAGMENT]);
    }

    if (Validated)
    {
        //SeparateUniformMVP = glGetUniformLocation(SeparateProgramName[program::VERTEX], "MVP");
        //SeparateUniformDiffuse = glGetUniformLocation(SeparateProgramName[program::FRAGMENT], "Diffuse");
    }

    return Validated && CheckError("initProgram");
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
    if (!useSep)
    {
        programID = CreateVSTessFSProgram("SepVertexShader.vert", "SepTcsShader.tesc",
                                            "SepTesShader.tese", "SepFragmentShader.frag" );
    }
    else
    {
        InitSeparateProgram();
    }

    static const GLfloat g_vertex_buffer_data[] =
    { 
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,
    };
    
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    if (enWireFrame)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glPatchParameteri(GL_PATCH_VERTICES, 3);
    float level = 5.0f;
    //glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, &level);
    //glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, &level);

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
    if (!useSep)
    {
        glUseProgram(programID);
    }
    else
    {
        glUseProgram(0);
        glBindProgramPipeline(PipelineName);
    }

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
    glDrawArrays(GL_PATCHES, 0, 3);

    glDisableVertexAttribArray(0);

    if (!useSep)
    {
        glUseProgram(0);
    }
    else
    {
        glBindProgramPipeline(0);
    }
}


void DeInitGL(void)
{
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);

    if (!useSep)
    {
        glDeleteProgram(programID);
    }
    else
    {
        glDeleteProgramPipelines(1, &PipelineName);
    }

    return;
}
