/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <osal.h>

#define USECS_PER_SEC     1000000

int osal_usleep (uint32 usec)
{
	return 0;
}

int osal_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	return 0;
}

ec_timet osal_current_time(void)
{
}

void osal_time_diff(ec_timet *start, ec_timet *end, ec_timet *diff)
{
}

void osal_timer_start(osal_timert * self, uint32 timeout_usec)
{
}

boolean osal_timer_is_expired (osal_timert * self)
{
	return 1;
}

void *osal_malloc(size_t size)
{
   return malloc(size);
}

void osal_free(void *ptr)
{
   free(ptr);
}

