/*
 * arp_out.c
 *
 *  Created on: Nov 26, 2010
 *      Author: amaar
 */

#include "finstypes.h"
#include "arp.h"

/**@brief This function is only activated once the cache already has the MAC address of the sought IP address. It sends out a Fins control frame to the ethernet stub
 * @param sought_IP_addrs is the IP address whose associated MAC address is sought
*/
void arp_out_ctrl(uint32_t sought_IP_addrs){

	struct finsFrame fins_arp_out;
	unsigned char *fins_MAC_address = (unsigned char *) malloc(sizeof(unsigned char)*HDWADDRSLEN);

	struct node *ptr_mac = ptr_cacheHeader;

	fins_arp_out.destinationID.id = ETHERSTUBID;
	fins_arp_out.dataOrCtrl = CONTROL;
	fins_arp_out.ctrlFrame.senderID = ARPID;
	fins_arp_out.ctrlFrame.opcode = READREPLY;
	MAC_addrs_conversion(search_MAC_addrs(sought_IP_addrs, ptr_mac), fins_MAC_address);
	fins_arp_out.ctrlFrame.paramterValue = fins_MAC_address;

	output_arp_queue(fins_arp_out);

}

/**@brief This function sends out a fins frame that passes an arp request out to the network
 * @param sought_IP_addrs is the IP address whose associated MAC address is sought
*/
void arp_out_request(uint32_t sought_IP_addrs){

	struct arp_hdr *pckt_arp = (struct arp_hdr*) malloc(sizeof(struct arp_hdr));
	struct ARP_message *arp_msg = (struct ARP_message *)malloc(sizeof(struct ARP_message));

	struct finsFrame fins_arp_out;

	gen_requestARP(sought_IP_addrs, arp_msg);
	arp_msg_to_hdr(arp_msg, pckt_arp);
	host_to_net(pckt_arp);

	fins_arp_out.destinationID.id = (unsigned char) ETHERSTUBID;
	fins_arp_out.dataOrCtrl = DATA;
	fins_arp_out.dataFrame.pdu = (unsigned char *)(pckt_arp);
	fins_arp_out.dataFrame.directionFlag= DOWN;
	fins_arp_out.dataFrame.pduLength = sizeof(struct arp_hdr);

	free(arp_msg); //free the temporarily used ARP message memory
	output_arp_queue(fins_arp_out);
}

/**@brief This function sends out a fins frame with a reply arp in response to a request
 * from the network
 * @param sender_IP_addrs is the IP address of the intended host
 * @param sender_MAC_addrs is the MAC address of the intended host
 * */
void arp_out_reply(uint32_t sender_IP_addrs, uint64_t sender_MAC_addrs){

	struct arp_hdr *pckt_arp = (struct arp_hdr*) malloc(sizeof(struct arp_hdr));
	struct ARP_message *arp_msg = (struct ARP_message *)malloc(sizeof(struct ARP_message));

	struct finsFrame fins_arp_out;

	gen_replyARP(sender_IP_addrs, sender_MAC_addrs, arp_msg);
	arp_msg_to_hdr(arp_msg, pckt_arp);
	host_to_net(pckt_arp);

	fins_arp_out.destinationID.id = (unsigned char) ETHERSTUBID;
	fins_arp_out.dataOrCtrl = DATA;
	fins_arp_out.dataFrame.pdu = (unsigned char *)(pckt_arp);
	fins_arp_out.dataFrame.directionFlag= DOWN;
	fins_arp_out.dataFrame.pduLength = sizeof(struct arp_hdr);
	free(arp_msg); //free the temporarily used ARP message

	output_arp_queue(fins_arp_out);
}


/**@brief to be completed. A fins frame is written to the 'wire'
 * @param fins_arp_out is the fins frame which will be written to the module-to-switch queue*/

void output_arp_queue(struct finsFrame fins_arp_out)
{

}

