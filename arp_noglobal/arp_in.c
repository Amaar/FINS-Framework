/*
 * arp_in_out.c
 *
 *  Created on: Oct 18, 2010
 *      Author: Syed Amaar Ahmad
 */

#include <stdio.h>
#include <stdlib.h>
#include<inttypes.h>
#include "finstypes.h"
#include "arp.h"
#include "finsdebug.h"
#include "metadata.h"


/**@brief this function receives an arp message from outside and processes it
 * @param fins_arp_in is the pointer to the fins frame which has been received by the ARP module
 */
void arp_in(struct finsFrame *fins_arp_in){

	struct ARP_message *arp_msg = (struct ARP_message*)malloc(sizeof(struct ARP_message));
	struct arp_hdr *pckt_arp;
	uint32_t target_IP_addrs;

	/**request or reply received from the network and as transmitted by the ethernet stub*/
	if (fins_arp_in->dataOrCtrl == DATA && (fins_arp_in->destinationID.id == ARPID))
	{

		pckt_arp = malloc(fins_arp_in->dataFrame.pduLength);
		memcpy(pckt_arp, fins_arp_in->dataFrame.pdu, fins_arp_in->dataFrame.pduLength);
		free(fins_arp_in->dataFrame.pdu); //free the memory used by arp header in the fins frame's pdu

		host_to_net(pckt_arp);               //convert it into the right format (e.g. htons issue etc.)
		arp_hdr_to_msg(pckt_arp, arp_msg);  /**convert the arp hdr into an internal arp message (e.g. use uint64_t instead of unsigned char pointer)*/
		free(pckt_arp);

		if (check_valid_arp(arp_msg)==1){

			update_cache(arp_msg); //add the sender's IP/MAC information to the ARP table

			if ((arp_msg->target_IP_addrs==interface_IP_addrs) && (arp_msg->operation==ARPREQUESTOP))
				arp_out_reply(arp_msg->sender_IP_addrs, arp_msg->sender_MAC_addrs);

			else if ((arp_msg->target_IP_addrs==interface_IP_addrs) && (arp_msg->operation==ARPREPLYOP))
			{		target_IP_addrs = arp_msg->sender_IP_addrs;
					arp_out_ctrl(target_IP_addrs); //generate fins control carrying target MAC address for the ethernet stub
			}
			free(arp_msg); //memory usage no longer needed
		}
	}
	else if ((fins_arp_in->dataOrCtrl == CONTROL) && (fins_arp_in->ctrlFrame.opcode == READREQUEST)
			&& (fins_arp_in->destinationID.id == ARPID))
	{/**request received from the ethernet stub-- IP address is provided in the fins control frame*/

		target_IP_addrs = fins_arp_in->ctrlFrame.paramterID;//IP address passed as unsigned integer

		/**request initiated by the ethernet stub*/
		if (search_list(ptr_cacheHeader, target_IP_addrs)==0)//i.e. not found
			arp_out_request(target_IP_addrs);
		else
			arp_out_ctrl(target_IP_addrs);//generate fins control carrying MAC address for ethernet stub
	}

}


