#ifndef __MACROS_H
#define __MACROS_H

#define __THROW_ERROR(err, __error__...) printf(__error__);
#define __THROW_FATAL_ERROR(err, __error__...) \
    printf(__error__);                         \
    getchar();                                 \
    getchar();                                 \
    exit(err);

void PauseIfGLError(const char *file, int line_num, const char *code);

#define GL_COMMAND(cmd) \
    cmd;                \
    PauseIfGLError(__FILE__, __LINE__, #cmd)

#endif