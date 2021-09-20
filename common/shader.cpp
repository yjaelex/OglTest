#include <stdio.h>
#include <vector>

#include "shader.hpp"
#include "main.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// --------------------------------------------------------------------------------------------------------------------
std::string FileContentsToString(std::string _filename)
{
    std::string retBuffer;

    FILE* file = 0;
    fopen_s(&file, _filename.c_str(), "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        size_t bytes = ftell(file);

        retBuffer.resize(bytes + 1);

        fseek(file, 0, SEEK_SET);
        fread(&(*retBuffer.begin()), 1, bytes, file);
        fclose(file);
        file = 0;
        return retBuffer;
    }

    error("Unable to locate file '%s'", _filename.c_str());

    return retBuffer;
}


bool CheckProgram(GLuint ProgramName)
{
    if (!ProgramName)
        return false;

    GLint Result = GL_FALSE;
    glGetProgramiv(ProgramName, GL_LINK_STATUS, &Result);

    if (Result == GL_TRUE)
        return true;

    //fprintf(stdout, "Linking program\n");
    int InfoLogLength;
    glGetProgramiv(ProgramName, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> Buffer(max(InfoLogLength, int(1)));
        glGetProgramInfoLog(ProgramName, InfoLogLength, NULL, &Buffer[0]);
        log("%s\n", &Buffer[0]);
    }

    return Result == GL_TRUE;
}

// --------------------------------------------------------------------------------------------------------------------
GLuint CompileShaderFromFile(GLenum _shaderType, std::string _shaderFilename, std::string _shaderPrefix)
{
    std::string shaderFullPath = _shaderFilename;

    const char* shaderPrefix = _shaderPrefix.c_str();

    GLuint retVal = glCreateShader(_shaderType);
    std::string fileContents = FileContentsToString(shaderFullPath);
    const char* includePath = ".";

    // GLSL has this annoying feature that the #version directive must appear first. But we 
    // want to inject some #define shenanigans into the shader. 
    // So to do that, we need to split for the part of the shader up to the end of the #version line,
    // and everything after that. We can then inject our defines right there.
    //auto strTuple = versionSplit(fileContents);
    //std::string versionStr = std::get<0>(strTuple);
    //std::string shaderContents = std::get<1>(strTuple);

    const char* shaderStrings[] = {
        // versionStr.c_str(),
        // "\n",
        // _shaderPrefix.c_str(),
        // "\n",
        fileContents.c_str()
    };

    glShaderSource(retVal, ArraySize(shaderStrings), shaderStrings, nullptr);
    glCompileShader(retVal);

    GLint compileStatus = 0;
    glGetShaderiv(retVal, GL_COMPILE_STATUS, &compileStatus);

    GLint glinfoLogLength = 0;
    glGetShaderiv(retVal, GL_INFO_LOG_LENGTH, &glinfoLogLength);
    if (glinfoLogLength > 1) {
        GLchar* buffer = new GLchar[glinfoLogLength];
        glGetShaderInfoLog(retVal, glinfoLogLength, &glinfoLogLength, buffer);
        if (compileStatus != GL_TRUE) {
            warn("Shader Compilation failed for shader '%s', with the following errors:", _shaderFilename.c_str());
        } else {
            log("Shader Compilation succeeded for shader '%s', with the following log:", _shaderFilename.c_str());
        }

        log("%s", buffer);
        delete[] buffer;
    }

    if (compileStatus != GL_TRUE) {
        glDeleteShader(retVal);
        retVal = 0;
    }

    return retVal;
}

static GLuint LinkProgram(GLuint retVal)
{
    glLinkProgram(retVal);

    GLint linkStatus = 0;
    glGetProgramiv(retVal, GL_LINK_STATUS, &linkStatus);

    GLint glinfoLogLength = 0;
    glGetProgramiv(retVal, GL_INFO_LOG_LENGTH, &glinfoLogLength);

    if (glinfoLogLength > 1) {
        GLchar* buffer = new GLchar[glinfoLogLength];
        glGetProgramInfoLog(retVal, glinfoLogLength, &glinfoLogLength, buffer);
        if (linkStatus != GL_TRUE) {
            warn("Shader Linking failed with the following errors:");
        }
        else {
            log("Shader Linking succeeded, with following warnings/messages:\n");
        }

        log("%s", buffer);
        delete[] buffer;
    }

    if (linkStatus != GL_TRUE) {
        error("Shader failed linking, here's an assert to break you in the debugger.");
        glDeleteProgram(retVal);
        retVal = 0;
    }

    return retVal;
}

// --------------------------------------------------------------------------------------------------------------------
static GLuint LinkVSFSShaders(GLuint _vs, GLuint _fs)
{
    GLuint retVal = glCreateProgram();
    glAttachShader(retVal, _vs);
    glAttachShader(retVal, _fs);

    retVal = LinkProgram(retVal);
    if (!retVal) {
        error("VS&FS link failed.");
    }

    return retVal;
}

static GLuint LinkVSGSFSShaders(GLuint _vs, GLuint _gs, GLuint _fs)
{
    GLuint retVal = glCreateProgram();
    glAttachShader(retVal, _vs);
    glAttachShader(retVal, _gs);
    glAttachShader(retVal, _fs);

    retVal = LinkProgram(retVal);
    if (!retVal) {
        error("VS&FS link failed.");
    }

    return retVal;
}

static GLuint LinkVSTessFSShaders(GLuint _vs, GLuint _tcs, GLuint _tes, GLuint _fs)
{
    GLuint retVal = glCreateProgram();
    glAttachShader(retVal, _vs);
    glAttachShader(retVal, _tes);
    glAttachShader(retVal, _tcs);
    glAttachShader(retVal, _fs);

    retVal = LinkProgram(retVal);
    if (!retVal) {
        error("VS&FS link failed.");
    }

    return retVal;
}

static GLuint LinkVSTessGSFSShaders(GLuint _vs, GLuint _tcs, GLuint _tes, GLuint _gs, GLuint _fs)
{
    GLuint retVal = glCreateProgram();
    glAttachShader(retVal, _vs);
    glAttachShader(retVal, _tes);
    glAttachShader(retVal, _gs);
    glAttachShader(retVal, _tcs);
    glAttachShader(retVal, _fs);

    retVal = LinkProgram(retVal);
    if (!retVal) {
        error("VS&FS link failed.");
    }

    return retVal;
}

// --------------------------------------------------------------------------------------------------------------------
GLuint CreateProgram(const std::string& _vsFilename, const std::string& _psFilename)
{
    return CreateProgram(_vsFilename, _psFilename, std::string(""));
}

// --------------------------------------------------------------------------------------------------------------------
GLuint CreateProgram(const std::string& _vsFilename, const std::string& _psFilename, const std::string& _shaderPrefix)
{
    GLuint vs = CompileShaderFromFile(GL_VERTEX_SHADER, _vsFilename, _shaderPrefix),
           fs = CompileShaderFromFile(GL_FRAGMENT_SHADER, _psFilename, _shaderPrefix);

    // If any are 0, dump out early.
    if ((vs * fs) == 0) {
        return 0;
    }

    GLuint retProgram = LinkVSFSShaders(vs, fs);

    glDeleteShader(fs);
    glDeleteShader(vs);

    return retProgram;
}

GLuint CreateVSGSFSProgram(const std::string& _vsFilename, const std::string& _gsFilename, const std::string& _psFilename)
{
    std::string _shaderPrefix = "";
    GLuint vs = CompileShaderFromFile(GL_VERTEX_SHADER, _vsFilename, _shaderPrefix),
           gs = CompileShaderFromFile(GL_GEOMETRY_SHADER, _gsFilename, _shaderPrefix),
           fs = CompileShaderFromFile(GL_FRAGMENT_SHADER, _psFilename, _shaderPrefix);

    // If any are 0, dump out early.
    if ((vs * gs * fs) == 0)
    {
        return 0;
    }

    GLuint retProgram = LinkVSGSFSShaders(vs, gs, fs);

    glDeleteShader(fs);
    glDeleteShader(gs);
    glDeleteShader(vs);

    return retProgram;
}


GLuint CreateVSTessFSProgram(const std::string& _vsFilename, const std::string& _tcsFilename,
                            const std::string& _tesFilename, const std::string& _psFilename)
{
    std::string _shaderPrefix = "";
    GLuint vs = CompileShaderFromFile(GL_VERTEX_SHADER, _vsFilename, _shaderPrefix),
           tcs = CompileShaderFromFile(GL_TESS_CONTROL_SHADER, _tcsFilename, _shaderPrefix),
           tes = CompileShaderFromFile(GL_TESS_EVALUATION_SHADER, _tesFilename, _shaderPrefix),
           fs = CompileShaderFromFile(GL_FRAGMENT_SHADER, _psFilename, _shaderPrefix);

    // If any are 0, dump out early.
    if ((vs * tcs * tes * fs) == 0) {
        return 0;
    }

    GLuint retProgram = LinkVSTessFSShaders(vs, tcs, tes, fs);

    glDeleteShader(fs);
    glDeleteShader(tes);
    glDeleteShader(tcs);
    glDeleteShader(vs);

    return retProgram;
}


GLuint CreateVSTessGSFSProgram(const std::string& _vsFilename, const std::string& _tcsFilename, const std::string& _tesFilename,
                               const std::string& _gsFilename, const std::string& _psFilename)
{
    std::string _shaderPrefix = "";
    GLuint vs = CompileShaderFromFile(GL_VERTEX_SHADER, _vsFilename, _shaderPrefix),
           tcs = CompileShaderFromFile(GL_TESS_CONTROL_SHADER, _tcsFilename, _shaderPrefix),
           tes = CompileShaderFromFile(GL_TESS_EVALUATION_SHADER, _tesFilename, _shaderPrefix),
           gs = CompileShaderFromFile(GL_GEOMETRY_SHADER, _gsFilename, _shaderPrefix),
           fs = CompileShaderFromFile(GL_FRAGMENT_SHADER, _psFilename, _shaderPrefix);

    // If any are 0, dump out early.
    if ((vs * tcs * tes * gs * fs) == 0) {
        return 0;
    }

    GLuint retProgram = LinkVSTessGSFSShaders(vs, tcs, tes, gs, fs);

    glDeleteShader(fs);
    glDeleteShader(gs);
    glDeleteShader(tes);
    glDeleteShader(tcs);
    glDeleteShader(vs);

    return retProgram;
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
	return CreateProgram(string(vertex_file_path), string(fragment_file_path));
}


bool ValidateProgramPipeline(GLuint pipelineName)
{
    GLint Status(0);
    glValidateProgramPipeline(pipelineName);
    glGetProgramPipelineiv(pipelineName, GL_VALIDATE_STATUS, &Status);

    if (Status != GL_TRUE)
    {
        GLint LengthMax(0);
        glGetProgramPipelineiv(pipelineName, GL_INFO_LOG_LENGTH, &LengthMax);

        GLsizei LengthQuery(0);
        std::vector<GLchar> InfoLog(LengthMax + 1, '\0');
        glGetProgramPipelineInfoLog(pipelineName, GLsizei(InfoLog.size()), &LengthQuery, &InfoLog[0]);
        log("ProgramPipeline VALIDATE FAILED, with following messages:\n");
        log("%s", &InfoLog[0]);

        //glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 76, GL_DEBUG_SEVERITY_LOW, LengthQuery, &InfoLog[0]);
        return false;
    }
    return true;
}
