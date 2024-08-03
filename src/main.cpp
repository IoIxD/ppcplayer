// #include "SDL.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include "GL/glut.h"
#include "player.hpp"
#include <Navigation.h>
#include "console/Console.hpp"

void PauseIfGLError(char *time, char *file, int line_num, char *code)
{
    auto err = glGetError();
    switch (err)
    {
    case GL_NO_ERROR:
        return;
    case GL_INVALID_ENUM:
        printf("[%s] Error on line %s:%d \"%s\": Invalid Enum\n", time, file, line_num, code);
        break;
    case GL_INVALID_VALUE:
        printf("[%s] Error on line %s:%d \"%s\": Invalid Value\n", time, file, line_num, code);
        break;
    case GL_INVALID_OPERATION:
        printf("[%s] Error on line %s:%d \"%s\": Invalid Operation\n", time, file, line_num, code);
        break;
    case GL_OUT_OF_MEMORY:
        printf("[%s] Error on line %s:%d \"%s\": Out of Memory\n", time, file, line_num, code);
        break;
    case GL_STACK_UNDERFLOW:
        printf("[%s] Error on line %s:%d \"%s\": Stack Underflow\n", time, file, line_num, code);
        break;
    case GL_STACK_OVERFLOW:
        printf("[%s] Error on line %s:%d \"%s\": Stack Overflow\n", time, file, line_num, code);
        break;
    }
    int x = getchar();
    printf("%d", x);
    int y = getchar();
    printf("%d", y);
    exit(err);
}

#define GL_COMMAND(cmd) \
    cmd;                \
    PauseIfGLError(__TIME__, __FILE__, __LINE__, #cmd)

static player::Player *pl = NULL;

void display(void);
void idle(void);
GLuint textureID;

int main(int argc, char **argv)
{

    printf("Type a file to open: \n", argc);
    char *buf;
    scanf("%s", buf);

    pl = new player::Player(buf);

    /* start of glut windowing and control functions */
    GL_COMMAND(glutInit(&argc, argv));
    GL_COMMAND(glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH));
    GL_COMMAND(glutInitWindowSize(pl->pRGBFrame->width, pl->pRGBFrame->height));

    GL_COMMAND(glutPositionWindow(640, 480));

    GL_COMMAND(glutCreateWindow(buf));

    GL_COMMAND(glutIdleFunc(idle));
    GL_COMMAND(glutDisplayFunc(display));

    GL_COMMAND(glutMainLoop());

    printf("Program exit.\n");
    getchar();
    return 0;
}

void UpdateTexture(GLsizei width, GLsizei height, uint8_t *data)
{
    // glActiveTexture(GL_TEXTURE);
    GL_COMMAND(glGenTextures(1, &textureID));
    GL_COMMAND(glBindTexture(GL_TEXTURE_2D, textureID));
    GL_COMMAND(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
    GL_COMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_COMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void idle(void)
{
    pl->step();
    GL_COMMAND(glutPostRedisplay());
}

void display(void)
{
    GL_COMMAND(glClearColor(1.0f, 0.0f, 1.0f, 1.0f));
    GL_COMMAND(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL_COMMAND(glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL));
    GL_COMMAND(glPushMatrix());
    GL_COMMAND(glTranslatef(0, 0, 0));

    auto frame = pl->pRGBFrame;
    UpdateTexture(frame->width, frame->height, frame->data[0]);

    GL_COMMAND(glBindTexture(GL_TEXTURE_2D, textureID));

    GL_COMMAND(glEnable(GL_TEXTURE_2D));

    GL_COMMAND(glBegin(GL_QUADS));
    GL_COMMAND(glTexCoord2f(0.0, 1.0));
    GL_COMMAND(glVertex2f(-1.0, -1.0));
    GL_COMMAND(glTexCoord2f(1.0, 1.0));
    GL_COMMAND(glVertex2f(1.0, -1.0));
    GL_COMMAND(glTexCoord2f(1.0, 0.0));
    GL_COMMAND(glVertex2f(1.0, 1.0));
    GL_COMMAND(glTexCoord2f(0.0, 0.0));
    GL_COMMAND(glVertex2f(-1.0, 1.0));
    GL_COMMAND(glEnd());

    GL_COMMAND(glutSwapBuffers());
    GL_COMMAND(glPopMatrix());
}
