/**
 * Retro68, as of writing, does not have usleep. We do this in its place.
 */

#include <unistd.h>

static QElem what;

#include <Timer.h>

void usleep_setup()
{
    InsTime(&what);
}

/**
 * Sleep USECONDS microseconds.
 */
int usleep(useconds_t usec)
{
    PrimeTime(&what, usec / 1000);
}