#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
using namespace std;

#include <GL/glew.h>

enum type
{
	VERTEX,
	TESS_CONTROL,
	TESS_EVALUATION,
	GEOMETRY,
	FRAGMENT,
	MAX
};

GLuint CreateProgram(const string& _vsFilename, const string& _psFilename);
GLuint CreateProgram(const string& _vsFilename, const string& _psFilename, const string& _shaderPrefix);
GLuint CreateVSTessGSFSProgram(const std::string& _vsFilename, const std::string& _tcsFilename, const std::string& _tesFilename,
                               const std::string& _gsFilename, const std::string& _psFilename);
GLuint CreateVSTessFSProgram(const std::string& _vsFilename, const std::string& _tcsFilename,
                            const std::string& _tesFilename, const std::string& _psFilename);
GLuint CreateVSGSFSProgram(const std::string& _vsFilename, const std::string& _gsFilename, const std::string& _psFilename);
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
std::string FileContentsToString(std::string _filename);
bool CheckProgram(GLuint ProgramName);
bool ValidateProgramPipeline(GLuint pipelineName);

#endif
