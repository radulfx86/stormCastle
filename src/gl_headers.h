#ifndef _GL_HEADERS_H_
#define _GL_HEADERS_H_
#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES3/gl3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#endif
#endif // _GL_HEADERS_H_