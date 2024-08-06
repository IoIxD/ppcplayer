#include "macros.h"
#include "GL/gl.h"
#include <stdio.h>
#include <stdlib.h>

void PauseIfGLError(const char *file, int line_num, const char *code)
{
    auto err = glGetError();
    switch (err)
    {
    case GL_NO_ERROR:
        return;
    case GL_INVALID_ENUM:
        __THROW_ERROR(err, "OpenGL Error at %s:%d \"%s\": Invalid Enum\n", file, line_num, code);
        break;
    case GL_INVALID_VALUE:
        __THROW_ERROR(err, "OpenGL Error at %s:%d \"%s\": Invalid Value\n", file, line_num, code);
        break;
    case GL_INVALID_OPERATION:
        __THROW_ERROR(err, "OpenGL Error at %s:%d \"%s\": Invalid Operation\n", file, line_num, code);
        break;
    case GL_OUT_OF_MEMORY:
        __THROW_ERROR(err, "OpenGL Error at %s:%d \"%s\": Out of Memory\n", file, line_num, code);
        break;
    case GL_STACK_UNDERFLOW:
        __THROW_ERROR(err, "OpenGL Error at %s:%d \"%s\": Stack Underflow\n", file, line_num, code);
        break;
    case GL_STACK_OVERFLOW:
        __THROW_ERROR(err, "OpenGL Error at %s:%d \"%s\": Stack Overflow\n", file, line_num, code);
        break;
    }
}
