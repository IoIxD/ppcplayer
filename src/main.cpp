extern "C"
{
#include "macros.h"
}
// #include "SDL.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include "GL/glut.h"
#include "player.hpp"
// #include "console/Console.hpp"
#include <stdio.h>
#include "ff2gl.h"

#ifdef __RETRO68__
#include "apple/usleep.h"
#else
#include <unistd.h>
#endif

static player::Player *pl = NULL;

void display(void);
void idle(void);
GLuint textureID;
int window;

GLint format = GL_RGB;
void InitTexture(GLsizei width, GLsizei height, uint8_t *data);

int main(int argc, char **argv)
{
#ifdef __RETRO68__
    usleep_setup();
#endif
    printf("Type a file to open: \n");
    char buf[255];
    scanf("%s", &buf);

    pl = new player::Player(buf);

    /* start of glut windowing and control functions */
    GL_COMMAND(glutInit(&argc, argv));
    GL_COMMAND(glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH));

    if (pl->hasVideo)
    {
        GL_COMMAND(glutInitWindowSize(pl->realWidth, pl->realHeight));

        auto frame = pl->pRGBFrame;
        InitTexture(frame->width, frame->height, frame->data[0]);

        format = ffmpeg_pix_format_to_gl(pl->pRGBFrame->format);
    }
    else
    {
        GL_COMMAND(glutInitWindowSize(512, 384));
    }

    window = GL_COMMAND(glutCreateWindow(buf));

    GL_COMMAND(glutSetWindowTitle(buf));

    GL_COMMAND(glutIdleFunc(idle));
    GL_COMMAND(glutDisplayFunc(display));

    GL_COMMAND(glutMainLoop());

    printf("Program exit.\n");
    getchar();
    return 0;
}

void InitTexture(GLsizei width, GLsizei height, uint8_t *data)
{

    // glActiveTexture(GL_TEXTURE);
    GL_COMMAND(glGenTextures(1, &textureID));
    GL_COMMAND(glBindTexture(GL_TEXTURE_2D, textureID));
    GL_COMMAND(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
    GL_COMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_COMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void UpdateTexture(GLsizei width, GLsizei height, uint8_t *data)
{
    // GL_COMMAND(glGenTextures(1, &textureID));
    GL_COMMAND(glBindTexture(GL_TEXTURE_2D, textureID));
    GL_COMMAND(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
    GL_COMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_COMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void idle(void)
{
    auto framerate = pl->framerate();
    pl->step();
    GL_COMMAND(glutPostRedisplay());
    usleep(framerate);
}

void display_hasVideo(void)
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

    glBegin(GL_QUADS);

    glTexCoord2f(0.0, pl->heightDiff);
    glVertex2f(-1.0, -1.0);
    glTexCoord2f(pl->widthDiff, pl->heightDiff);
    glVertex2f(1.0, -1.0);
    glTexCoord2f(pl->widthDiff, 0.0);
    glVertex2f(1.0, 1.0);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(-1.0, 1.0);
    glEnd();

    GL_COMMAND(glutSwapBuffers());
    GL_COMMAND(glPopMatrix());
}

void display_onlyAudio(void)
{
    GL_COMMAND(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));

    GL_COMMAND(glutSwapBuffers());
}

void display(void)
{
    if (pl->hasVideo)
    {
        display_hasVideo();
    }
    else
    {
        display_onlyAudio();
    }
}
