/*
 * switch.c
 *
 *  Created on: Nov 4, 2010
 *      Author: amaar
 */
#include <stdio.h>
#include <stdlib.h>
#include "finstypes.h"
#include "switch.h"
#include "queueModule.h"
#include "finsdebug.h"
#include "metadata.h"

#define DEBUG

/**@brief initializes the switch and the switch-to-module and module-to-switch queues (including the semaphrores).
 * Each queue pointer corresponds to a unidirectional flow of fins frames and is predeclared
 * in the switch.h header file.
 * @param file_name is the name of the configuration files storing the table records
 * */
void init_switch(char *file_name){

	FILE *ptr_to_config_file;
	ptr_to_config_file =fopen(file_name,"r");  //open file

	if (!ptr_to_config_file)  // exit if file cannot open
		exit(0);

	//switch_table_ptr is the Switch's internal pointer to its table records
	switch_table_ptr = read_table(file_name);  /**initialize the table from config file*/
	fclose(ptr_to_config_file);

	/**Switch initializes all queue buffers between itself and modules.
	 * Each module (which has 2 unidirectional queues to and from the switch),
	 * however, needs to be aware of which queue it reads from and which queue
	 * it writes to. It (i.e. init_queue()) also initializes the semaphores controlling thread access to the queues
	 * */
	ip_to_swt_bff = init_queue();
	swt_to_ip_bff = init_queue();
	tcp_to_swt_bff = init_queue();
	swt_to_tcp_bff = init_queue();
	wifi_to_swt_bff = init_queue();
	swt_to_wifi_bff = init_queue();
	arp_to_swt_bff = init_queue();
	swt_to_arp_bff = init_queue();
}

/**@brief This function closes the file of records, frees memory allocated to the switch and the queues
 */
void term_switch(){
	free_records(switch_table_ptr);
	term_queue(ip_to_swt_bff);
	term_queue(swt_to_ip_bff);
	term_queue(tcp_to_swt_bff);
	term_queue(swt_to_tcp_bff);
	term_queue(wifi_to_swt_bff);
	term_queue(swt_to_wifi_bff);
	term_queue(arp_to_swt_bff);
	term_queue(swt_to_arp_bff);
}

/**@brief This function generates the rely
 * @param reply is the pointer to reply fins frame for query
 * @param request is the pointer to request fins frame to query
 * @param searched_table_ptr is the pointer to the linked list as generated from the table
 */

void gen_query_reply(struct finsFrame *reply, struct finsFrame *request, struct tableRecord *searched_table_ptr){

	/**search_table_ptr points to the first element of the linked list searched from the table*/

	reply->dataOrCtrl=request->dataOrCtrl;
	reply->ctrlFrame.opcode= QUERYREPLY;
	reply->ctrlFrame.serialNum = request->ctrlFrame.serialNum;
	reply->ctrlFrame.senderID = request->destinationID.id;
	reply->destinationID.id = request->ctrlFrame.senderID;
	reply->destinationID.next = NULL;
	reply->ctrlFrame.paramterValue = request->ctrlFrame.paramterValue;
	reply->ctrlFrame.paramterID = request->ctrlFrame.paramterID;
	reply->ctrlFrame.replyRecord = searched_table_ptr;
}

/**@brief This function generates the linked list of records based on the search
  * @param request is the pointer to request fins frame to query
 */

struct tableRecord * switch_search_query(struct finsFrame *request){

	struct tableRecord *ptr, *ptr2; /**<temporary pointers for iterating purposes*/
	int found;
	struct tableRecord *first_rec;

	found = 0;
	ptr = switch_table_ptr;  /** points to the global pointer to the Table which the switch can access*/

	while (ptr!=NULL) // while the Table has not been entirely read keep looping
	{

		if (request->ctrlFrame.senderID == ptr->sourceID)
		{

			if (found==0)
			{
				first_rec = (struct tableRecord *) malloc (sizeof(struct tableRecord));
				first_rec->sourceID = ptr->sourceID;
				first_rec->destinationID = ptr->destinationID;
				first_rec->vci = ptr->vci;
				first_rec->directionFlag = ptr->directionFlag;
				first_rec->next = NULL;
				ptr2 = first_rec;
			}
			else if (found>0)
			{
				struct tableRecord *later_rec = (struct tableRecord *) malloc (sizeof(struct tableRecord));

				later_rec->sourceID = ptr->sourceID;
				later_rec->destinationID = ptr->destinationID;
				later_rec->vci = ptr->vci;
				later_rec->directionFlag = ptr->directionFlag;
				later_rec->next = NULL;

				ptr2->next = later_rec;
				ptr2 = later_rec;
			}

			found = found+1;
		}

		ptr=ptr->next;

	}//end while loop


	if (found==0)
		return NULL; //no record found hence return NULL
	else
		return first_rec;  /**return pointer the first element
		of the new linked list containing the table records*/
}

