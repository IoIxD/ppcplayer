/**
 * Retro68, as of writing, does not have usleep. We do this in its place.
 */

#include <unistd.h>
#include <Timer.h>

static QElem what;

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