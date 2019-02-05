/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <machine/endian.h>
#include "oshw.h"
#include "intel_i210.h"

ec_adaptert	adapters [DEVS_MAX_NB];

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
	int i;
	if (eth_discover_devices() < 0)
		return NULL;	// ERROR
	for (i = 0;; ++i) {
		struct eth_device *dev = eth_get_device(i);
		if (dev == NULL) {
			adapters[i-1].next = NULL;
			break;
		}
		strncpy(adapters[i].name, dev->name, MAX_DEVICE_NAME);
		adapters[i].next = &adapters[i+1];
	}
	return &(adapters[0]);
}

/** Free memory allocated memory used by adapter collection.
 * @param[in] adapter = First element in linked list of adapters
 * EC_NOFRAME.
 */
void oshw_free_adapters(ec_adaptert *adapter)
{
}

extern int ec_slavecount;

void print_slave_info (void)
{
	OSEE_PRINT("Printing slave info for %d slaves...\n", ec_slavecount);
   	for (int i = 0; i < ec_slavecount; ++i) {
	   	OSEE_PRINT("Name: %s\n", ec_slave[i].name);
	   	OSEE_PRINT("\tState: %u\n", ec_slave[i].state);
	   	OSEE_PRINT("\tOutput bytes: %u\n", ec_slave[i].Obytes);
	   	OSEE_PRINT("\tInput bytes: %u\n", ec_slave[i].Ibytes);
	   	OSEE_PRINT("\tConfigured address: %u\n\n", ec_slave[i].configadr);
	}
}
