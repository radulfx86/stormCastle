#ifndef _IO_H_
#define _IO_H_
#include <string>
#include <fstream>
#include <sstream>
#include "gl_headers.h"

std::string loadText(const std::string &path)
{
    std::ifstream str(path, std::ios::in);
    std::stringstream sstr;
    sstr << str.rdbuf();
    return sstr.str();
}

GLuint loadTexture(const char *path, GLuint offset)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

#ifdef __EMSCRIPTEN__
    int w, h;
    char *data = emscripten_get_preloaded_image_data(path, &w, &h);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    printf("loaded image of size %d x %d\n", w, h);
    free(data);
#else
    SDL_Surface *surface = IMG_Load(path);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    SDL_FreeSurface(surface);

#endif

    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST);


    return tex;
}
#endif // _IO_H_