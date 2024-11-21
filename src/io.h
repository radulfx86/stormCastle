#ifndef _IO_H_
#define _IO_H_
#include <string>
#include <fstream>
#include <sstream>
#include "gl_headers.h"

std::string loadText(const std::string &path);

GLuint loadTexture(const char *path, GLuint offset);

#endif // _IO_H_