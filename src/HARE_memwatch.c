#include "HARE_memwatch.h"

void initMemwatch(void)
{
#ifdef MEMWATCH
    mwInit();  // Initializes memwatch
#endif  // MEMWATCH
    return;
}

void abortMemwatch(void)
{
#ifdef MEMWATCH
    mwAbort();  // Terminate memwatch even if more than one call to mwTerm() is outstanding
#endif  // MEMWATCH
    return;
}

void termMemwatch(void)
{
#ifdef MEMWATCH
    mwTerm();  // Terminates memwatch
#endif  // MEMWATCH
    return;
}
