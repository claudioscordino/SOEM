/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <machine/endian.h>
#include "oshw.h"

#define MAX_ADAPTERS	3
ec_adaptert	adapters [MAX_ADAPTERS];

/**
 * Host to Network byte order (i.e. to big endian).
 *
 * Note that Ethercat uses little endian byte order, except for the Ethernet
 * header which is big endian as usual.
 */
inline uint16 oshw_htons(uint16 host)
{
	// __htons() is provided by the bare-metal x86 compiler
	return __htons(host);
}

/**
 * Network (i.e. big endian) to Host byte order.
 *
 * Note that Ethercat uses little endian byte order, except for the Ethernet
 * header which is big endian as usual.
 */
inline uint16 oshw_ntohs(uint16 network)
{
	// __ntohs() is provided by the bare-metal x86 compiler
	return __ntohs(network);
}

/** Create list over available network adapters.
 * @return First element in linked list of adapters
 */
ec_adaptert* oshw_find_adapters(void)
{
	// TODO: call eth_discover_devices()
	// if (devs_nb < 1)
	//	return NULL;
	// Iterate over devs[i] and fill as follows:
	//	sprintf(adapters[i].name, "%d\n", i); 
	//	sprintf(adapters[i].desc, "%d\n", i); 
	//	adapters[i].next = ...
	return &(adapters[0]);
}

/** Free memory allocated memory used by adapter collection.
 * @param[in] adapter = First element in linked list of adapters
 * EC_NOFRAME.
 */
void oshw_free_adapters(ec_adaptert *adapter)
{
}
