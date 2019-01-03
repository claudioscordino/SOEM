/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#ifndef _osal_defs_
#define _osal_defs_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/time.h>
#include <stdlib.h>

#ifndef PACKED
#define PACKED_BEGIN
#define PACKED  __attribute__((__packed__))
#define PACKED_END
#endif

int osal_gettimeofday(struct timeval *tv, struct timezone *tz);
void *osal_malloc(size_t size);
void osal_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
