// Include GLEW
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <common/shader.hpp>
#include <common/main.h>

#ifdef _WIN32
#include <common/_getopt.h>
#else
#ifdef LINUX
#include <getopt.h>
#endif
#endif

GLuint vertexbuffer = -1;
GLuint VertexArrayID = -1;
GLuint colorbuffer = -1;
GLuint elementsbuffer = -1;
GLuint elementCount = 0;
GLuint programID = -1;
glm::mat4 MVP;
GLuint uniformMVP = -1;
glm::mat4 MV;
GLuint uniformMV = -1;
glm::mat4 P;
GLuint uniformP = -1;
GLuint uniformTessLevel = -1;
GLuint uniformNormScale = -1;
GLuint ubo_cb0 = -1;
GLuint ubo_cb1 = -1;
GLuint ubo_cb2 = -1;
GLuint ubo_cb3 = -1;

static int enWireFrame = 0;
static int verboseFlag = 0;
static int useSep = 0;
static int useGS = 0, useUBO = 0, useTess = 0;

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

        // Use Uniform Block in VS/TCS/TES/GS
        {"ubo",     no_argument, 0, 'u'},
        // Enable GS; default pipeline is VS/FS
        {"gs",      no_argument, 0, 'g'},
        // Enable Tessellation.
        {"tess",    no_argument, 0, 't'},
        // Enable All features except separate shader objects.
        {"all",     no_argument, 0, 'a'},

        {"help",    no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int c = 0;

    while (1)
    {
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long(argc, argv, "aghstuv", long_options, &option_index);

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

        case 'g':
            useGS = 1;
            break;
        case 'u':
            useUBO = 1;
            break;
        case 't':
            useTess = 1;
            break;
        case 'a':
            useGS = useUBO = useTess = 1;
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
                "  --ubo, -u     : Use Uniform Block in VS/TCS/TES/GS\n"
                "  --gs, -g      : Enable GS; default pipeline is VS/FS\n"
                "  --tess, -t    : Enable Tessellation.\n"
                "  --all, -a     : Enable All features except separate shader objects.\n"
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
GLuint PipelineName = -1;

bool InitSeparateProgram()
{
    bool Validated = true;
    log("Init Separate Program!\n");

    std::string vsFile = "SimpleVertexShader.vert";
    std::string tcsFile = "";
    std::string tesFile = "";
    std::string gsFile = "";
    if (!useUBO)
    {
        if (!useGS)
        {
            if (!useTess)
            {
            }
            else
            {
                vsFile = "VS.vert";
                tcsFile = "Tcs.tesc";
                tesFile = "Tes.tese";
            }
        }
        else
        {
            if (!useTess)
            {
                vsFile = "VS.vert";
                gsFile = "GS.geom";
            }
            else
            {
                vsFile = "VS.vert";
                tcsFile = "Tcs.tesc";
                tesFile = "TesForGS.tese";
                gsFile = "GS.geom";
            }
        }
    }
    else
    {
        if (!useGS)
        {
            if (!useTess)
            {
                vsFile = "UBOSimpleVS.vert";
            }
            else
            {
                vsFile = "UBOVS.vert";
                tcsFile = "UBOTcs.tesc";
                tesFile = "UBOTes.tese";
            }
        }
        else
        {
            if (!useTess)
            {
                vsFile = "UBOVS.vert";
                gsFile = "UBOGS.geom";
            }
            else
            {
                vsFile = "UBOVS.vert";
                tcsFile = "UBOTcs.tesc";
                tesFile = "UBOTesForGS.tese";
                gsFile = "UBOGS.geom";
            }
        }
    }

    glGenProgramPipelines(1, &PipelineName);

    if (Validated)
    {
        std::string VertexSourceContent = FileContentsToString(vsFile);
        char const* VertexSourcePointer = VertexSourceContent.c_str();
        SeparateProgramName[VERTEX] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &VertexSourcePointer);
    }

    if (Validated && useTess)
    {
        std::string TcsSourceContent = FileContentsToString(tcsFile);
        char const* TcsSourcePointer = TcsSourceContent.c_str();
        SeparateProgramName[TESS_CONTROL] = glCreateShaderProgramv(GL_TESS_CONTROL_SHADER, 1, &TcsSourcePointer);
        CheckProgram(SeparateProgramName[TESS_CONTROL]);
        glUseProgramStages(PipelineName, GL_TESS_CONTROL_SHADER_BIT, SeparateProgramName[TESS_CONTROL]);
    }

    if (Validated && useTess)
    {
        std::string TesSourceContent = FileContentsToString(tesFile);
        char const* TesSourcePointer = TesSourceContent.c_str();
        SeparateProgramName[TESS_EVALUATION] = glCreateShaderProgramv(GL_TESS_EVALUATION_SHADER, 1, &TesSourcePointer);
        CheckProgram(SeparateProgramName[TESS_EVALUATION]);
        glUseProgramStages(PipelineName, GL_TESS_EVALUATION_SHADER_BIT, SeparateProgramName[TESS_EVALUATION]);
    }

    if (Validated && useGS)
    {
        std::string GeometrySourceContent = FileContentsToString(gsFile);
        char const* GeometrySourcePointer = GeometrySourceContent.c_str();
        SeparateProgramName[GEOMETRY] = glCreateShaderProgramv(GL_GEOMETRY_SHADER, 1, &GeometrySourcePointer);
        CheckProgram(SeparateProgramName[GEOMETRY]);
        glUseProgramStages(PipelineName, GL_GEOMETRY_SHADER_BIT, SeparateProgramName[GEOMETRY]);
    }

    if (Validated)
    {
        std::string FragmentSourceContent = FileContentsToString("SimpleFragmentShader.frag");
        char const* FragmentSourcePointer = FragmentSourceContent.c_str();
        SeparateProgramName[FRAGMENT] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FragmentSourcePointer);
    }

    if (Validated)
    {
        Validated = Validated && CheckProgram(SeparateProgramName[VERTEX]);
        Validated = Validated && CheckProgram(SeparateProgramName[FRAGMENT]);
    }

    if (Validated)
    {
        glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, SeparateProgramName[VERTEX]);
        glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, SeparateProgramName[FRAGMENT]);
        ValidateProgramPipeline(PipelineName);
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
        if (!useUBO)
        {
            if (!useGS)
            {
                if (!useTess)
                {
                    programID = LoadShaders("SimpleVertexShader.vert", "SimpleFragmentShader.frag");
                }
                else
                {
                    programID = CreateVSTessFSProgram("VS.vert", "Tcs.tesc",
                        "Tes.tese", "SimpleFragmentShader.frag");
                }
            }
            else
            {
                if (!useTess)
                {
                    programID = CreateVSGSFSProgram("VS.vert", "GS.geom", "SimpleFragmentShader.frag");
                }
                else
                {
                    programID = CreateVSTessGSFSProgram("VS.vert", "Tcs.tesc", "TesForGS.tese",
                        "GS.geom", "SimpleFragmentShader.frag");
                }
            }
        }
        else
        {
            if (!useGS)
            {
                if (!useTess)
                {
                    programID = LoadShaders("UBOSimpleVS.vert", "SimpleFragmentShader.frag");
                }
                else
                {
                    programID = CreateVSTessFSProgram("UBOVS.vert", "UBOTcs.tesc",
                        "UBOTes.tese", "SimpleFragmentShader.frag");
                }
            }
            else
            {
                if (!useTess)
                {
                    programID = CreateVSGSFSProgram("UBOVS.vert", "UBOGS.geom", "SimpleFragmentShader.frag");
                }
                else
                {
                    programID = CreateVSTessGSFSProgram("UBOVS.vert", "UBOTcs.tesc", "UBOTesForGS.tese",
                        "UBOGS.geom", "SimpleFragmentShader.frag");
                }
            }
        }
    }
    else
    {
        InitSeparateProgram();
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Handle default UBO
	// Get a handle for our "MVP" uniform
    if (!useSep)
    {
        uniformMVP = glGetUniformLocation(programID, "MVP");
        uniformMV = glGetUniformLocation(programID, "MV");
        uniformP = glGetUniformLocation(programID, "P");
        uniformTessLevel = glGetUniformLocation(programID, "tessLevel");
        uniformNormScale = glGetUniformLocation(programID, "normScale");
    }
    else
    {
        uniformMVP = glGetUniformLocation(SeparateProgramName[VERTEX], "MVP");
        uniformMV = glGetUniformLocation(SeparateProgramName[VERTEX], "MV");
        if (useGS)
        {
            uniformP = glGetUniformLocation(SeparateProgramName[GEOMETRY], "P");
            uniformNormScale = glGetUniformLocation(SeparateProgramName[GEOMETRY], "normScale");
        }

        if (useTess)
        {
            if (!useGS)
            {
                uniformP = glGetUniformLocation(SeparateProgramName[TESS_EVALUATION], "P");
            }
            uniformTessLevel = glGetUniformLocation(SeparateProgramName[TESS_CONTROL], "tessLevel");
        }
    }

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View       = glm::lookAt(
								glm::vec3(4,3,-3),  // Camera is at (4,3,-3), in World Space
								glm::vec3(0,0,0),   // and looks at the origin
								glm::vec3(0,1,0)    // Head is up (set to 0,-1,0 to look upside-down)
						   );
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);
	// Our ModelViewProjection
    MV = View * Model;
    P = Projection;
	MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around


    /////////////////////////////////////////////////////////////////////////////////////////////
    // Handle UBO
    if (useUBO)
    {
        GLuint proID = programID;

        if (useSep)
        {
            proID = SeparateProgramName[VERTEX];
        }
        GLuint uniformBlockSize = 0;
        GLuint cb0 = glGetUniformBlockIndex(proID, "CB0");  // VS
        if (cb0 == GL_INVALID_INDEX)
        {
            log("UBO CB0: glGetUniformBlockIndex returns GL_INVALID_INDEX.\n");
        }
        else
        {
            glUniformBlockBinding(proID, cb0, 1);
            glm::vec3 cb0_diffuse = glm::vec3(0.2, 0.4, 0.6);
            uniformBlockSize = sizeof(cb0_diffuse);
            glGenBuffers(1, &ubo_cb0);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_cb0);
            glBufferData(GL_UNIFORM_BUFFER, uniformBlockSize, glm::value_ptr(cb0_diffuse), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_cb0);
        }

        if (useSep)
        {
            proID = SeparateProgramName[GEOMETRY];
        }
        GLuint cb1 = glGetUniformBlockIndex(proID, "CB1");  // GS
        if (cb1 == GL_INVALID_INDEX)
        {
            log("UBO CB1: glGetUniformBlockIndex returns GL_INVALID_INDEX.\n");
        }
        else
        {
            glUniformBlockBinding(proID, cb1, 2);
            glm::vec3 cb1_new = glm::vec3(0.6, 0.01, 0.1);
            uniformBlockSize = sizeof(cb1_new);
            glGenBuffers(1, &ubo_cb1);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_cb1);
            glBufferData(GL_UNIFORM_BUFFER, uniformBlockSize, glm::value_ptr(cb1_new), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo_cb1);
        }

        if (useSep)
        {
            proID = SeparateProgramName[TESS_CONTROL];
        }
        GLuint cb2 = glGetUniformBlockIndex(proID, "CB2");  // TCS Color
        if (cb2 == GL_INVALID_INDEX)
        {
            log("UBO CB2: glGetUniformBlockIndex returns GL_INVALID_INDEX.\n");
        }
        else
        {
            glUniformBlockBinding(proID, cb2, 3);
            glm::vec3 cb2_color = glm::vec3(0.1, 0.2, 0.5);
            uniformBlockSize = sizeof(cb2_color);
            glGenBuffers(1, &ubo_cb2);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_cb2);
            glBufferData(GL_UNIFORM_BUFFER, uniformBlockSize, glm::value_ptr(cb2_color), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            glBindBufferBase(GL_UNIFORM_BUFFER, 3, ubo_cb2);
        }

        if (useSep)
        {
            proID = SeparateProgramName[TESS_EVALUATION];
        }
        GLuint cb3 = glGetUniformBlockIndex(proID, "CB3");  // TES
        if (cb3 == GL_INVALID_INDEX)
        {
            log("UBO CB3: glGetUniformBlockIndex returns GL_INVALID_INDEX.\n");
        }
        else
        {
            glUniformBlockBinding(proID, cb3, 4);
            GLfloat cb3_scale = 0.7;
            uniformBlockSize = sizeof(cb3_scale);
            glGenBuffers(1, &ubo_cb3);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_cb3);
            glBufferData(GL_UNIFORM_BUFFER, uniformBlockSize, &(cb3_scale), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            glBindBufferBase(GL_UNIFORM_BUFFER, 4, ubo_cb3);
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    static const GLfloat g_vertex_buffer_data[] = {
        /* front */
        -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        /* back */
        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0,
    };

    static const GLfloat g_color_buffer_data[] = {
        /*front colors*/
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
        /* back colors*/
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
    };

    static const GLuint g_elements_data[] = {
        /*front*/
        0, 1, 2,
        2, 3, 0,
        /*top*/
        1, 5, 6,
        6, 2, 1,
        /*back*/
        7, 6, 5,
        5, 4, 7,
        /*bottom*/
        4, 0, 3,
        3, 7, 4,
        /*left*/
        4, 5, 1,
        1, 0, 4,
        /*right*/
        3, 2, 6,
        6, 7, 3
    };

    elementCount = sizeof(g_elements_data) / sizeof(GLuint);

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    glGenBuffers (1, &elementsbuffer);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementsbuffer);
	glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (g_elements_data), g_elements_data, GL_STATIC_DRAW );

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(
        1,                                // attribute 1. must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // 1st attribute buffer : pos
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0. must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // Must enable Z test, otherwize the cube will be ugly!!!
    glEnable (GL_DEPTH_TEST);
    if (enWireFrame)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (useTess)
    {
        glPatchParameteri(GL_PATCH_VERTICES, 3);
    }

    return true;
}

void ReSizeGLScene(size_t Width, size_t Height)
{
    glViewport(0, 0, GLsizei(Width), GLsizei(Height));
}

void DrawGLScene(void)
{
    static float32 rotation = 0.03f;
    static glm::mat4 r = glm::mat4(1.0f);	// holds a rotation
    r = glm::rotate(r, glm::radians(float32(rotation)), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 m = glm::mat4(1.0f);

    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glBindVertexArray(VertexArrayID);
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

    if (uniformMVP != -1)
    {
        if (useSep)
        {
            glActiveShaderProgram(PipelineName, SeparateProgramName[VERTEX]);
        }

        m = MVP * r;
        glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(m));
    }
    if (uniformMV != -1)
    {
        if (useSep)
        {
            glActiveShaderProgram(PipelineName, SeparateProgramName[VERTEX]);
        }

        m = MV * r;
        glUniformMatrix4fv(uniformMV, 1, GL_FALSE, glm::value_ptr(m));
    }
    if (uniformP != -1)
    {
        if (useSep)
        {
            GLuint program = useGS ? SeparateProgramName[GEOMETRY] : SeparateProgramName[TESS_EVALUATION];
            glActiveShaderProgram(PipelineName, program);
        }

        glUniformMatrix4fv(uniformP, 1, GL_FALSE, glm::value_ptr(P));
    }
    if (uniformTessLevel != -1)
    {
        if (useSep)
        {
            glActiveShaderProgram(PipelineName, SeparateProgramName[TESS_CONTROL]);
        }
        glUniform2f(uniformTessLevel, 8.0, 4.0);
    }
    if (uniformNormScale != -1)
    {
        if (useSep)
        {
            glActiveShaderProgram(PipelineName, SeparateProgramName[GEOMETRY]);
        }

        GLfloat tmp = useTess ? 0.2 : 0.5;
        glUniform1f(uniformNormScale, tmp);
    }

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Draw the cube
    if (!useTess)
    {
        glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawElements(GL_PATCHES, elementCount, GL_UNSIGNED_INT, 0);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
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
    glDeleteProgram(programID);

    if (ubo_cb0 != -1)
    {
        glDeleteBuffers(1, &ubo_cb0);
    }
    if (ubo_cb1 != -1)
    {
        glDeleteBuffers(1, &ubo_cb1);
    }
    return;
}
