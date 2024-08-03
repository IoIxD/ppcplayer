// #include "SDL.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include "GL/glut.h"
#include "player.hpp"
#include <Navigation.h>
#include "console/Console.hpp"

static player::Player *pl = NULL;

void display(void);
void idle(void);
GLuint textureID;

int main(int argc, char **argv)
{

    printf("Type a file to open: \n", argc);
    char *buf;
    scanf("%s", buf);
    delete retro::Console::currentInstance;

    pl = new player::Player(buf);

    /* start of glut windowing and control functions */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(pl->pRGBFrame->width, pl->pRGBFrame->height);

    glutPositionWindow(640, 480);

    glutCreateWindow(buf);

    glutIdleFunc(idle);
    glutDisplayFunc(display);

    glutMainLoop();

    printf("Program exit.\n");
    getchar();
    return 0;
}

void UpdateTexture(GLsizei width, GLsizei height, uint8_t *data)
{
    // glActiveTexture(GL_TEXTURE);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void idle(void)
{
    pl->step();
    glutPostRedisplay();
}

void display(void)
{
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glPushMatrix();
    glTranslatef(0, 0, 0);

    auto frame = pl->pRGBFrame;
    UpdateTexture(frame->width, frame->height, frame->data[0]);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0);
    glVertex2f(-1.0, -1.0);
    glTexCoord2f(1.0, 1.0);
    glVertex2f(1.0, -1.0);
    glTexCoord2f(1.0, 0.0);
    glVertex2f(1.0, 1.0);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(-1.0, 1.0);
    glEnd();

    glutSwapBuffers();
    glPopMatrix();
}

void __display(void)
{
    // clear the screen and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    printf("decode\n");
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);

    auto frame = pl->pRGBFrame;

    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width,
                    frame->height, GL_RGB, GL_UNSIGNED_BYTE,
                    frame->data[0]);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(-1.0, -1.0);
    glTexCoord2f(1.0, 0.0);
    glVertex2f(1.0, -1.0);
    glTexCoord2f(1.0, 1.0);
    glVertex2f(1.0, 1.0);
    glTexCoord2f(0.0, 1.0);
    glVertex2f(-1.0, 1.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
}
/*

// I don't trust assert, the compiler disables the message on release mode and I don't know
// if my program is actually being compiled in debug mode.
#define __GUARANTEED_ESCAPE_IF_NULL(__val__)           \
    if (__val__ == NULL)                               \
    {                                                  \
        printf("Loop error: " #__val__ " == NULL;\n"); \
        \
                        ret = AVERROR(EACCES);         \
        int ch = getchar();                            \
        printf("%c", ch);                              \
        goto end;                                      \
    }

#define __SDL_ERROR_IF_NULL(__val__)                                     \
    if (__val__ == NULL)                                                 \
    {                                                                    \
        printf("SDL Error creating " #__val__ ": %s\n", SDL_GetError()); \
        int ch = getchar();                                              \
        printf("%c", ch);                                                \
        goto end;                                                        \
    }
    SDL_Event event;

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_WM_SetCaption(buf, NULL);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
    int amask = 0x000000ff;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
    int amask = 0xff000000;
#endif

    SDL_Surface *surface = SDL_SetVideoMode(videoPar->width, videoPar->height, 24, SDL_HWSURFACE);

    __SDL_ERROR_IF_NULL(surface);

        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());

                    SDL_Rect rect = (SDL_Rect){0, 0, pRGBFrame->width, pRGBFrame->height};

                    int err = SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 255, 0));

                        printf("SDL Error: %s\n", SDL_GetError());

    SDL_Quit();

*/