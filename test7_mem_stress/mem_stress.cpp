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

#include <vector>

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

std::vector<GLuint> vUBOId;
GLfloat* g_pData = NULL;
const GLuint uboSize = 16 * 1024 * 1024;     // 16 MB
GLuint totalUboSize = uboSize;
GLuint allocLoopCount = 1;

static int enWireFrame = 0;
static int verboseFlag = 0;

void ProcessCommandLine(int argc, char* argv[])
{
    static struct option long_options[] =
    {
        /* These options set a flag. */
        // "--verbose" long option just set verboseFlag=1
        {"verbose", no_argument, &verboseFlag, 1},
        {"lines",   no_argument, &enWireFrame, 1},

        // Allocate this size of video memory
        {"size",     required_argument, 0, 's'},

        {"help",    no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int c = 0;
    GLuint num = 0;

    while (1)
    {
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long(argc, argv, "hs", long_options, &option_index);

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

        case 's':
            if (optarg)
            {
                log("Will allocate %s MB video memory!", optarg);
                num = atoi(optarg);
            }
            else
            {
                num = 1024 * 1024 * 1024;   // 1GB
            }
            if (totalUboSize < num)
            {
                totalUboSize = num;
            }
            break;
        case 'v':
            verboseFlag = 1;
            break;

        case 'h':
            error("Options:\n"
                "  --lines       : Enable WireFrame mode.\n"
                "  --verbose, -v : Verbose mode.\n"
                "  --size, -s     : Allocate size of MB video memory. Default is 16MB.\n"
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


static GLuint UpdateUBO(GLuint size, GLfloat* pData)
{
    GLuint proID = programID;
    GLuint ubo = -1;
    GLuint cb0 = glGetUniformBlockIndex(proID, "CB0");  // VS
    if (cb0 == GL_INVALID_INDEX)
    {
        log("UBO CB0: glGetUniformBlockIndex returns GL_INVALID_INDEX.\n");
    }
    else
    {
        glUniformBlockBinding(proID, cb0, 1);
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, size, pData, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);
    }

    //GLuint cb1 = glGetUniformBlockIndex(proID, "CB1");  // FS
    //if (cb1 == GL_INVALID_INDEX)
    //{
    //    log("UBO CB1: glGetUniformBlockIndex returns GL_INVALID_INDEX.\n");
    //}
    //else
    //{
    //    glUniformBlockBinding(proID, cb1, 2);
    //    glm::vec3 cb1_new = glm::vec3(0.6, 0.01, 0.1);
    //    uniformBlockSize = sizeof(cb1_new);
    //    glGenBuffers(1, &ubo_cb1);
    //    glBindBuffer(GL_UNIFORM_BUFFER, ubo_cb1);
    //    glBufferData(GL_UNIFORM_BUFFER, uniformBlockSize, glm::value_ptr(cb1_new), GL_DYNAMIC_DRAW);
    //    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo_cb1);
    //}

    return ubo;
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
    programID = LoadShaders("VS.vert", "FS.frag");

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Handle default UBO
	// Get a handle for our "MVP" uniform

    uniformMVP = glGetUniformLocation(programID, "MVP");
    uniformMV = glGetUniformLocation(programID, "MV");
    uniformP = glGetUniformLocation(programID, "P");

	// Projection matrix : 45?? Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
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

    allocLoopCount = totalUboSize / uboSize + 1;
    g_pData = (GLfloat*)malloc(uboSize);
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

    glUseProgram(programID);

    static GLuint index = 0;
    static float32 angle = 0.3f;
    if (index < allocLoopCount)
    {
        g_pData[0] = angle;
        angle += 0.2f;
        GLuint id = UpdateUBO(uboSize, g_pData);
        if (id != -1)
        {
            vUBOId.push_back(id);
        }
        index++;
    }

    if (uniformMVP != -1)
    {
        m = MVP * r;
        glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(m));
    }
    if (uniformMV != -1)
    {
        m = MV * r;
        glUniformMatrix4fv(uniformMV, 1, GL_FALSE, glm::value_ptr(m));
    }
    if (uniformP != -1)
    {
        glUniformMatrix4fv(uniformP, 1, GL_FALSE, glm::value_ptr(P));
    }

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Draw the cube
    glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glUseProgram(0);
}

void DeInitGL(void)
{
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);

    free(g_pData);

    for (uint32 i = 0; i < vUBOId.size(); i++)
    {
        glDeleteBuffers(1, &vUBOId[i]);
    }

    return;
}
