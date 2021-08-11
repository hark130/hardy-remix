#ifndef _HARE_MEMWATCH_H_
#define _HARE_MEMWATCH_H_

#ifdef MEMWATCH
#include "memwatch.h"
#endif  // MEMWATCH

/*
 * HARE_memwatch.h
 * Always include HARE_memwatch.h in each module to facilitate
 * Memwatch testing.
 * Call the HARE Memwatch functions as listed:
 *  initMemwatch() -  At the start of your program
 *  abortMemwatch() - If the program is stopping due to an error
 *  termMemwatch() -  At the logical no-error end of your program
 *
 * DETAILS:
 * From https://github.com/viruscamp/memwatch/blob/master/FAQ:
 *
 * "You USUALLY don't need to call mwInit() and mwTerm()..."
 * "You can call mwInit() and mwTerm() manually, if it's not initializing properly..."
 * SPOILER ALERT: It's not
 * "Call mwInit() as soon as you can, and mwTerm() at
 * the logical no-error ending of your program. Call mwAbort()
 * if the program is stopping due to an error; this will
 * terminate memwatch even if more than one call to mwTerm() is
 * outstanding."
 */

/*
 * function: initMemwatch
 * description: Manually initializes Memwatch
 *
 * notes:
 *  Conditionally compiled with the MEMWATCH macro
 *  "Call [initMemwatch] as soon as you can..."
 *  See header file comment block for more details/insight
 * return: None
 */
void initMemwatch(void);

/*
 * function: abortMemwatch
 * description: Manually aborts Memwatch
 *
 * notes:
 *  Conditionally compiled with the MEMWATCH macro
 *  "Call [abortMemwatch] if the program is stopping due to an error; this will
 *      terminate memwatch even if more than one call to mwTerm() is outstanding."
 *  See header file comment block for more details/insight
 * return: None
 */
void abortMemwatch(void);

/*
 * function: termMemwatch
 * description: Manually terminates Memwatch
 *
 * notes:
 *  Conditionally compiled with the MEMWATCH macro
 *  "Call [termMemwatch] at the logical no-error ending of your program."
 *  See header file comment block for more details/insight
 * return: None
 */
void termMemwatch(void);

#endif  // _HARE_MEMWATCH_H_
