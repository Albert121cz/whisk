// This file was sourced from:
// https://gist.github.com/Plasmoxy/aec637b85e306f671339dcfd509efc82
// (also edited)

// Header for GLDebugMessageCallback by Plasmoxy 2020
// Feel free to use this in any way.
// #pragma once
#ifndef GL_DEBUG_MESSAGE_CALLBACK_H_
#define GL_DEBUG_MESSAGE_CALLBACK_H_

#include <GL/glew.h>
#include <wx/wx.h>
// #include <GLFW/glfw3.h>

namespace oglDebug
{
    extern void GLAPIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length,
        const GLchar* msg, const void* data);
}

#endif /* GL_DEBUG_MESSAGE_CALLBACK_H */