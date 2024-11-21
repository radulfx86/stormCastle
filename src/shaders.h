#ifndef _SHADER_S_H_
#define _SHADER_S_H_
#include "gl_headers.h"

int getShaderCompileInfo(GLuint shader);

int getShaderLinkInfo(GLuint program);

GLuint createShader(const char *vertexShader, const char *fragmentShader);

#endif // _SHADER_S_H_