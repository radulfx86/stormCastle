#include "gl_headers.h"
#include "shaders.h"
#include <string>

int getShaderCompileInfo(GLuint shader)
{
    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if ( not ok )
    {
        printf("shader failed to compile\n");
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if ( infoLen > 1 )
        {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen+1);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf("ERROR: \n%s\n", infoLog);
            free(infoLog);
        }
    }
    else
    {
        printf("shader compiled successfully: %d\n", shader);
    }
    return ok;
}

int getShaderLinkInfo(GLuint program)
{
    int ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if ( not ok )
    {
        printf("shader failed to link\n");
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if ( infoLen > 1 )
        {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen+1);
            glGetProgramInfoLog(program, infoLen, NULL, infoLog);
            printf("ERROR: \n%s\n", infoLog);
            free(infoLog);
        }
    }
    else
    {
        printf("shader linked successfully: %d\n", program);
    }
    return ok;
}

GLuint createShader(const char *vertexShader, const char *fragmentShader)
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, NULL);
    glCompileShader(vs);
    getShaderCompileInfo(vs);
    
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, NULL);
    glCompileShader(fs);
    getShaderCompileInfo(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    getShaderLinkInfo(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}