#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#if defined(__has_include)
  #if __has_include(<SDL2/SDL_opengl.h>)
    #include <SDL2/SDL_opengl.h>
    #include <SDL2/SDL_opengl_glext.h>
  #elif __has_include(<SDL_opengl.h>)
    #include <SDL_opengl.h>
    #include <SDL_opengl_glext.h>
  #else
    #include <GL/gl.h>
  #endif
#else
  #include <SDL_opengl.h>
  #include <SDL_opengl_glext.h>
#endif
#endif
