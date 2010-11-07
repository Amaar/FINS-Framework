/**
 * switch_in_out.c
 *@brief
 * @date Created on: Nov 3, 2010
 *  @author: amaar ahmad
 */
#include <stdio.h>
#include <stdlib.h>
#include "finstypes.h"
#include "queueModule.h" //this has header files on the queue pointers and semaphores
#include "switch.h"
#include "finsdebug.h"
#include "metadata.h"

#define DEBUG

/**@brief from the (any) module to switch queue a fins frame is read into the
 * switch. Note that the function init_switch() is needed to initialize the switch, the queues and the semaphores which
 * control access to these queues. This function reads from the queue as pointed to by the input parameter and sends
 * a fins frame in response via the fins_switch_out.
 * @param module_to_switch_bff is the pointer to the queue from (any) module into the switch
 */
void fins_switch_in(struct queue *module_to_switch_bff){

	struct finsFrame fins_in, fins_out;
	struct destinationList *dstPtr;
	struct tableRecord *searched_table_ptr;

	sem_wait(&(module_to_switch_bff->locked));/**lock the queue being read from*/
	read_queue(&fins_in, module_to_switch_bff);
	sem_post(&(module_to_switch_bff->locked));/**lock the queue being read from*/

	if (fins_in.dataOrCtrl == CONTROL)  //control type fins Frame
	{

		if (fins_in.ctrlFrame.opcode == QUERYREQUEST)  //if this is a query request generate a query reply
		{
			searched_table_ptr = switch_search_query(&(fins_in)); //carries the request fins frame
			gen_query_reply(&fins_out, &fins_in, searched_table_ptr);
			print_records(searched_table_ptr); //pointer to linked list of found elements from the table
		}

		//for all other opcodes simply copy the frame and send out
		else if ((fins_in.ctrlFrame.opcode == READREQUEST)
			&&(fins_in.ctrlFrame.opcode == READREPLY)
			&& (fins_in.ctrlFrame.opcode == WRITEREQUEST)
			&& (fins_in.ctrlFrame.opcode == WRITECONF))

			memcpy(&fins_out, &fins_in, sizeof(struct finsFrame));

			fins_switch_out(&fins_out);
	}
	else if (fins_in.dataOrCtrl == DATA)  //data type fins Frame
	{
		/**@check the destination ID and send the frame to the module with this ID*/

		dstPtr = &(fins_in.destinationID);

/**@ The received FINS frame is duplicated and send to each of the modules (i.e. dest list).
 * However the destination list is pruned for the correct member for this purpose.
 * For example, if the list has three members each of the three modules will receive exactly
 * the same frame but without the original destination list (i.e. the destination ID will contain only the module's ID).
 * */

		memcpy(&fins_out, &fins_in, sizeof(struct finsFrame));

		while (dstPtr!=NULL){

			fins_out.destinationID.id = dstPtr->id;
			fins_out.destinationID.next = NULL;
			dstPtr = dstPtr->next;
			fins_switch_out(&fins_out); //send to the queue for the given module as stored in dstPtr
		}
	}
}

/**@brief performs the task of writing to a queue from the Fins switch to the module
 * @param fins_out is the pointer to the fins frame being written to a switch-to-module queue
 */
void fins_switch_out(struct finsFrame *fins_out){

	/**Pick the right queue to write the fins frame to. Semaphore lock and unlock is performed to
	 * protect the integrity of the queue. Note that the fins frame is written to the right switch-to-module queue.*/

//	print_finsFrame(fins_out); //print Fins frame

	if (fins_out->destinationID.id==ARPID){
		sem_wait(&(swt_to_arp_bff->locked));
		write_queue(fins_out, swt_to_arp_bff);
		sem_post(&(swt_to_arp_bff->locked));
	}
	else if (fins_out->destinationID.id==TCPID){
		sem_wait(&(swt_to_tcp_bff->locked));
		write_queue(fins_out, swt_to_tcp_bff);
		sem_post(&(swt_to_tcp_bff->locked));
	}
	else if (fins_out->destinationID.id==IPID){
		sem_wait(&(swt_to_ip_bff->locked));
		write_queue(fins_out, swt_to_ip_bff);
		sem_post(&(swt_to_ip_bff->locked));
	}
	else if (fins_out->destinationID.id==ETHERSTUBID){
		sem_wait(&(swt_to_wifi_bff->locked));
		write_queue(fins_out, swt_to_wifi_bff);
		sem_post(&(swt_to_wifi_bff->locked));
	}

}
