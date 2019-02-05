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

#include <inttypes.h>

#include "ethercat.h"

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
   	for (int i = 1; i < ec_slavecount + 1; ++i) {
	   	OSEE_PRINT("Name: %s\n", ec_slave[i].name);
	   	OSEE_PRINT("\tSlave nb: %d\n", i);
		if ((ec_slave[i].state & 0x0f) == EC_STATE_NONE)
	   		OSEE_PRINT("\tState: NONE\n");
		else if ((ec_slave[i].state & 0x0f) == EC_STATE_INIT)
	   		OSEE_PRINT("\tState: INIT\n");
		else if ((ec_slave[i].state & 0x0f) == EC_STATE_PRE_OP)
	   		OSEE_PRINT("\tState: PRE_OP\n");
		else if ((ec_slave[i].state & 0x0f) == EC_STATE_BOOT)
	   		OSEE_PRINT("\tState: BOOT\n");
		else if ((ec_slave[i].state & 0x0f) == EC_STATE_SAFE_OP)
	   		OSEE_PRINT("\tState: SAFE_OP\n");
		else if ((ec_slave[i].state & 0x0f) == EC_STATE_OPERATIONAL)
	   		OSEE_PRINT("\tState: OPERATIONAL\n");
		if (ec_slave[i].state & 0x10)
	   		OSEE_PRINT("\tState: ACK or ERROR\n");
	   	OSEE_PRINT("\tOutput bytes: %u\n", ec_slave[i].Obytes);
	   	OSEE_PRINT("\tOutput bits: %u\n", ec_slave[i].Obits);
	   	OSEE_PRINT("\tInput bytes: %u\n", ec_slave[i].Ibytes);
	   	OSEE_PRINT("\tInput bits: %u\n", ec_slave[i].Ibits);
	   	OSEE_PRINT("\tConfigured address: %u\n", ec_slave[i].configadr);
	   	OSEE_PRINT("\tOutput address: %x\n", ec_slave[i].outputs);
	   	OSEE_PRINT("\tOstartbit: %x\n", ec_slave[i].Ostartbit);
	   	OSEE_PRINT("\tCoE details: %x\n", ec_slave[i].CoEdetails); // See ECT_COEDET_*
	   	OSEE_PRINT("\tHas DC capability: %d\n\n", ec_slave[i].hasdc);
	}
}

void set_operational (void)
{
    	int chk;
	int expectedWKC;

        ec_configdc();

        OSEE_PRINT("Slaves mapped, state to SAFE_OP.\n");
        /* wait for all slaves to reach SAFE_OP state */
        ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);


        OSEE_PRINT("segments : %d : %d %d %d %d\n",
			ec_group[0].nsegments,
			ec_group[0].IOsegment[0],
			ec_group[0].IOsegment[1],
			ec_group[0].IOsegment[2],
			ec_group[0].IOsegment[3]);

        OSEE_PRINT("Request operational state for all slaves\n");
        expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
        OSEE_PRINT("Calculated workcounter %d\n", expectedWKC);
        ec_slave[0].state = EC_STATE_OPERATIONAL;
        /* send one valid process data to make outputs in slaves happy*/
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        /* request OP state for all slaves */
        ec_writestate(0);
        chk = 40;
        /* wait for all slaves to reach OP state */
        do {
            	ec_send_processdata();
            	ec_receive_processdata(EC_TIMEOUTRET);
            	ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE * 4);
        } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
        ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE * 4);

        if (ec_slave[0].state == EC_STATE_OPERATIONAL ) {
            	OSEE_PRINT("Operational state reached for all slaves.\n");
	} else {
            	OSEE_PRINT("WARNING: Operational state NOT reached for all slaves.\n");
	}


        ec_slave[4].state = EC_STATE_OPERATIONAL;
        ec_writestate(4);
        ec_send_processdata();

	int16_t value = 0x3FFF;
	uint8 *data_ptr;
	data_ptr = ec_slave[4].outputs;
	*data_ptr++ = (value >> 0) & 0xFF;
   	*data_ptr++ = (value >> 8) & 0xFF;
        ec_send_processdata();
}


#if 0

char usdo[128];
char hstr[1024];

char* SDO2string(uint16 slave, uint16 index, uint8 subidx, uint16 dtype)
{
   int l = sizeof(usdo) - 1, i;
   uint8 *u8;
   int8 *i8;
   uint16 *u16;
   int16 *i16;
   uint32 *u32;
   int32 *i32;
   uint64 *u64;
   int64 *i64;
   float *sr;
   double *dr;
   char es[32];

   memset(&usdo, 0, 128);
   ec_SDOread(slave, index, subidx, FALSE, &l, &usdo, EC_TIMEOUTRXM);
   if (EcatError)
   {
      return ec_elist2string();
   }
   else
   {
      switch(dtype)
      {
         case ECT_BOOLEAN:
            u8 = (uint8*) &usdo[0];
            if (*u8) sprintf(hstr, "TRUE");
             else sprintf(hstr, "FALSE");
            break;
         case ECT_INTEGER8:
            i8 = (int8*) &usdo[0];
            sprintf(hstr, "0x%2.2x %d", *i8, *i8);
            break;
         case ECT_INTEGER16:
            i16 = (int16*) &usdo[0];
            sprintf(hstr, "0x%4.4x %d", *i16, *i16);
            break;
         case ECT_INTEGER32:
         case ECT_INTEGER24:
            i32 = (int32*) &usdo[0];
            sprintf(hstr, "0x%8.8x %d", *i32, *i32);
            break;
         case ECT_INTEGER64:
            i64 = (int64*) &usdo[0];
            sprintf(hstr, "0x%16.16"PRIx64" %"PRId64, *i64, *i64);
            break;
         case ECT_UNSIGNED8:
            u8 = (uint8*) &usdo[0];
            sprintf(hstr, "0x%2.2x %u", *u8, *u8);
            break;
         case ECT_UNSIGNED16:
            u16 = (uint16*) &usdo[0];
            sprintf(hstr, "0x%4.4x %u", *u16, *u16);
            break;
         case ECT_UNSIGNED32:
         case ECT_UNSIGNED24:
            u32 = (uint32*) &usdo[0];
            sprintf(hstr, "0x%8.8x %u", *u32, *u32);
            break;
         case ECT_UNSIGNED64:
            u64 = (uint64*) &usdo[0];
            sprintf(hstr, "0x%16.16"PRIx64" %"PRIu64, *u64, *u64);
            break;
         case ECT_REAL32:
            sr = (float*) &usdo[0];
            sprintf(hstr, "%f", *sr);
            break;
         case ECT_REAL64:
            dr = (double*) &usdo[0];
            sprintf(hstr, "%f", *dr);
            break;
         case ECT_BIT1:
         case ECT_BIT2:
         case ECT_BIT3:
         case ECT_BIT4:
         case ECT_BIT5:
         case ECT_BIT6:
         case ECT_BIT7:
         case ECT_BIT8:
            u8 = (uint8*) &usdo[0];
            sprintf(hstr, "0x%x", *u8);
            break;
         case ECT_VISIBLE_STRING:
            strcpy(hstr, usdo);
            break;
         case ECT_OCTET_STRING:
            hstr[0] = 0x00;
            for (i = 0 ; i < l ; i++)
            {
               sprintf(es, "0x%2.2x ", usdo[i]);
               strcat( hstr, es);
            }
            break;
         default:
            sprintf(hstr, "Unknown type");
      }
      return hstr;
   }
}




ec_ODlistt ODlist;
ec_OElistt OElist;
void si_sdo(int cnt)
{
    	int i, j;

    	ODlist.Entries = 0;
    	memset(&ODlist, 0, sizeof(ODlist));
    	if( ec_readODlist(cnt, &ODlist))
    	{
        	OSEE_PRINT(" CoE Object Description found, %d entries.\n",ODlist.Entries);
        	for( i = 0 ; i < ODlist.Entries ; i++)
        	{
            		ec_readODdescription(i, &ODlist);
            		while(EcatError) OSEE_PRINT("%s", ec_elist2string());
            		OSEE_PRINT(" Index: %4.4x Datatype: %4.4x Objectcode: %2.2x Name: %s\n",
                			ODlist.Index[i], ODlist.DataType[i], ODlist.ObjectCode[i], ODlist.Name[i]);
            		memset(&OElist, 0, sizeof(OElist));
            		ec_readOE(i, &ODlist, &OElist);
            		while(EcatError) OSEE_PRINT("%s", ec_elist2string());
            		for( j = 0 ; j < ODlist.MaxSub[i]+1 ; j++)
            		{
                		if ((OElist.DataType[j] > 0) && (OElist.BitLength[j] > 0))
                		{
                    			OSEE_PRINT("  Sub: %2.2x Datatype: %4.4x Bitlength: %4.4x Obj.access: %4.4x Name: %s\n",
                        				j, OElist.DataType[j], OElist.BitLength[j], OElist.ObjAccess[j], OElist.Name[j]);
                    			if ((OElist.ObjAccess[j] & 0x0007))
                    			{
                        			OSEE_PRINT("          Value :%s\n", SDO2string(cnt, ODlist.Index[i], j, OElist.DataType[j]));
                    			}
                		}
            		}
        	}
    	}
    	else
    	{
        	while(EcatError) OSEE_PRINT("%s", ec_elist2string());
    	}
}
#endif
