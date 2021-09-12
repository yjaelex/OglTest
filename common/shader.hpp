#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
using namespace std;

#include <GL/glew.h>

GLuint CreateProgram(const string& _vsFilename, const string& _psFilename);
GLuint CreateProgram(const string& _vsFilename, const string& _psFilename, const string& _shaderPrefix);
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
std::string FileContentsToString(std::string _filename);
bool CheckProgram(GLuint ProgramName);

#endif
