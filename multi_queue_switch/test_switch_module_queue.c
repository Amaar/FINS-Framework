/**
 * @author Syed Amaar Ahmad
 * @date Nov 3, 2010
 * This function is the main function which tests the switch/query operation using
 * queues of fins frames
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include "queueModule.h"
#include "module_query_ops.h"
#include "switch.h"
#include <semaphore.h>
#include <sys/sem.h>
#include <pthread.h>    /* POSIX Threads */
#include "finsdebug.h"
#include "metadata.h"

#define DEBUG

int main(int argc, char *argv[])
{
	unsigned int vc; /**<refers to the VCI for a module's internal search*/
	int d, i; /**<user defined input*/
	struct finsFrame write_fins, read_fins;/**<fins frame used for sending to and receiving messages from the switch*/
	struct destinationList *dest_list;
	unsigned char test_data;

	produce_table(argv[1]);  /** The file is populated with randomly generated table records.
	In practice a config file will be read by the switch.*/


	init_switch(argv[1]); //read config. file, initialize the switch and the queues (and their associated semaphores)


/**For tesing fins DATA frame via the switch. Suppose that the TCP module has to send some
 * data (test_data) to the IP module and ARP module*/

	test_data = 0x2f;

	write_fins.dataFrame.directionFlag = DOWN;
	write_fins.dataFrame.pdu = &test_data;
	write_fins.dataOrCtrl = DATA;
	write_fins.destinationID.id = IPID;
	write_fins.destinationID.next = NULL;
	struct destinationList *next_dest = (struct destinationList *) malloc(sizeof(struct destinationList));
	next_dest->id = ARPID;
	next_dest->next = NULL;
	write_fins.destinationID.next = next_dest;

	//write to the right queues which have already been initialized by the switch
	sem_wait(&(tcp_to_swt_bff->locked)); //lock the queue which is going to be written to (see init_switchh() inswitch.c)
	write_queue(&write_fins, tcp_to_swt_bff);
	sem_post(&(tcp_to_swt_bff->locked));//unlock the queue which has been written to

	fins_switch_in(tcp_to_swt_bff); //Fins switch called. TCP sends the fins frame to the switch which does the rest.

	sem_wait(&(swt_to_ip_bff->locked));
	read_queue(&read_fins, swt_to_ip_bff);//The switch has written to this queue and the IP module reads from it
	sem_post(&(swt_to_ip_bff->locked));
	PRINT_DEBUG("\nTCP has sent %x and the IP receives %x\n", test_data, *(unsigned char*)(read_fins.dataFrame.pdu));

	sem_wait(&(swt_to_arp_bff->locked));
	read_queue(&read_fins, swt_to_arp_bff);//ARP  module reads the fins frame from its switch-to-module queue
	sem_post(&(swt_to_arp_bff->locked));
	PRINT_DEBUG("\nTCP has sent %x and the ARP receives %x\n\n", test_data, *(unsigned char*)(read_fins.dataFrame.pdu));

	free(next_dest);

/**Now for tesing fins CONTROL frame via the switch. Suppose that a module has to send some query to the switch.
The switch needs to send a reply to a request based on the search from its table. This side acts like a given module
which is accessing the switch, sending query requests to it and updating its local cache based on the replies. It also
offers the capability to run through the local cache and determine the list of destinations based on VCI and direction flag.
Let us choose another module for testing this time.
 */

	for (i=0;i<1;i++){

	module_query_task(GENQUERY, IPID, &write_fins, i);//generate a query request from the module (now say IP module)

	//lock, write fins frame to queue, unlock
	sem_wait(&(ip_to_swt_bff->locked)); //lock the queue which is going to be written to
	write_queue(&write_fins, ip_to_swt_bff);
	sem_post(&(ip_to_swt_bff->locked));//unlock the queue which has been written to

	fins_switch_in(ip_to_swt_bff); /**fins switch is called that reads from the module-to-switch queue
									(and writes to another queue)*/

	//lock, read fins frame from queue, unlock...
	sem_wait(&(swt_to_ip_bff->locked));
	read_queue(&read_fins, swt_to_ip_bff);//switch has written a query reply to this queue. Module reads from it.
	sem_post(&(swt_to_ip_bff->locked));

	module_query_task(UPDATECACHE, TCPID, &read_fins, i);//a module can update the cache based on the reply as received from switch

	PRINT_DEBUG("\nQuery reply serial number %d\n", read_fins.ctrlFrame.serialNum);

	//now searching the local cache based on the VCI and direction flag
	PRINT_DEBUG("\nEnter VCI for destination list search based on the module's local cache\n ");
	scanf("%d", &vc);
	PRINT_DEBUG("\nEnter Direction Flag '1' for DOWN, '0' for UP \n ");
	scanf("%d", &d);

	if (d==1)
		dest_list = search_module_table(module_cache_ptr, (unsigned char) vc, DOWN);
	else if (d==0)
		dest_list = search_module_table(module_cache_ptr, (unsigned char) vc, UP);

	print_dest_list(dest_list);
	}

	//once the standalone testing is complete, free all memory used in the heap

	free_records(module_cache_ptr);//free module side cache
	free_dest_list(dest_list); //free destination list memory use
	term_switch(); //free all memory as used by the switch

	return 0;
}
