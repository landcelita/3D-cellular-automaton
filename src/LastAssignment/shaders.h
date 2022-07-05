#ifndef SHADERS_H
#define SHADERS_H

#include <glad/gl.h>


#include <string>
#include "common.h"

static std::string VERT_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.vert";
static std::string FRAG_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.frag";

GLuint compileShader(const std::string &filename, GLuint type);
GLuint buildShaderProgram(const std::string &vShaderFile, const std::string &fShaderFile);
GLuint initShaders();

#endif // SHADERS_H