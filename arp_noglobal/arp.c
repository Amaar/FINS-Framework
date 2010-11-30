/**@file arp.c
 *@brief this files contains all relevant functions to execute an ARP module,
 *@brief IP and MAC address of the host is provided by the main
 *@author Syed Amaar Ahmad
 *@date  September 27, 2010
 */

#include <stdio.h>
#include <stdlib.h>
#include<inttypes.h>
#include "finstypes.h"
#include "arp.h"
#include "finsdebug.h"
#include "metadata.h"

#define DEBUG

/**
 * An address like a.b.c.d (e.g. 5.45.0.07 where a= 5, b=45, c=0,d=7) is converted an integer
 * @brief this function takes a user defined address and produces a uint32 address
 * @param a an octet (most significant)
 * @param b an octet
 * @param c an octet
 * @param d an octet (least significant)
 */
uint32_t gen_IP_addrs(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{	return (16777216ul*(a) + (65536ul*(b)) + (256ul*(c)) + (d));
}

/**
 * An address like a:b:c:d:e:f is converted into an 64-byte unsigned integer
 * @brief this function takes a user provided MAC address as a set of octets and produces a uint64 address
 * @param a an octet (most significant)
 * @param b an octet
 * @param c an octet
 * @param d an octet
 * @param e an octet
 * @parm f an octet (least significant)*/

uint64_t gen_MAC_addrs(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
{return 1099511627776ull*(a)+ 4294967296ull*(b) + 16777216ull*(c) + 65536ull*(d) + (256ull*(e)) + (f);}


/**
 * @brief this function produces an ARP request for a host whose IP address is known
 * @param IP_address_target is the uint32 address of the target host
 * @param request_ARP_ptr is the pointer to the ARP request message that will be generated
 */
void gen_requestARP(uint32_t IP_address_target, struct ARP_message *request_ARP_ptr)
{
	request_ARP_ptr->sender_MAC_addrs = interface_MAC_addrs;
	request_ARP_ptr->sender_IP_addrs = interface_IP_addrs;
	request_ARP_ptr->target_MAC_addrs = NULLADDRESS;
	request_ARP_ptr->target_IP_addrs = IP_address_target;
	request_ARP_ptr->hardware_type = (HWDTYPE);
	request_ARP_ptr->protocol_type = (PROTOCOLTYPE);
	request_ARP_ptr->hardware_addrs_length= HDWADDRSLEN ;
	request_ARP_ptr->protocol_addrs_length = PROTOCOLADDRSLEN;
	request_ARP_ptr->operation = (ARPREQUESTOP);
}

/**
 * @brief this function produces an ARP reply for the host to a request for a MAC address
 * @param target_IP_addrs is the IP address of the requesting host
 * @param target_MAC_addrs is the MAC address of the requesting host
 * @param arp_msg is the pointer to the ARP message that will be sent out
 */

void gen_replyARP(uint32_t target_IP_addrs, uint64_t target_MAC_addrs, struct ARP_message *arp_msg)
{
	arp_msg->sender_MAC_addrs = interface_MAC_addrs;
	arp_msg->sender_IP_addrs = interface_IP_addrs;
	arp_msg->target_MAC_addrs = target_MAC_addrs;
	arp_msg->target_IP_addrs = target_IP_addrs;
	arp_msg->hardware_type = HWDTYPE;
	arp_msg->protocol_type = PROTOCOLTYPE;
	arp_msg->hardware_addrs_length= HDWADDRSLEN;
	arp_msg->protocol_addrs_length = PROTOCOLADDRSLEN;
	arp_msg->operation = ARPREPLYOP;
}

/**
 * @brief this function searches a list or cache for to check if it contains a
 * particular MAC address, returns 1 if found else returns 0
 * @param ptr_firstElementOfList is the pointer to the first element of the cache
 * @param IP_addrs is the IP address being searched in the cache
 */
int search_list(struct node *ptr_firstElementOfList, uint32_t IP_addrs)
{
	struct node *ptr_elementInList;
	int found = 0;
	ptr_elementInList = ptr_firstElementOfList;

	while (ptr_elementInList!=NULL)
	{
		if ((ptr_elementInList->IP_addrs == IP_addrs) && (ptr_elementInList->MAC_addrs!=NULLADDRESS))
			found = 1;

		ptr_elementInList = ptr_elementInList->next;
	}
	return found;
}

/** * ptr_cacheHeader is the pointer to the first element of the cache
 * @brief this function updates a cache based on an ARP message
 * @param pckt is an arp message (either reply or request) from some host within the neighborhood
 */
void update_cache(struct ARP_message *pckt)
{
	int found = 0;
	struct node *ptr_interfaceHeader;/**these variables are used to traverse the linked list structure of cache*/
	struct ARP_message *pckt_ARP;

	pckt_ARP = pckt;

	/**update cache is performed only is the received arp message pointer is not null*/
	if (check_valid_arp(pckt_ARP)==1)
	{
		/**cache header is a node which has information about the interface of
		 * the host itself. It links via 'next' pointer to the node containing addresses of
		 * a neighbor. Each subsequent neighbor is linked via this pointer
		 *
		 * */
		if (ptr_cacheHeader==NULL){
			struct node *new_host = (struct node *) malloc(sizeof(struct node));
			new_host->next = NULL;
			new_host->IP_addrs = pckt_ARP->sender_IP_addrs;
			new_host->MAC_addrs = pckt_ARP->sender_MAC_addrs;
			ptr_cacheHeader = new_host;
		}
		else{
			ptr_interfaceHeader = ptr_cacheHeader;

			/**returns 1 if found, else 0*/
			found = search_list(ptr_interfaceHeader, pckt_ARP->sender_IP_addrs);
			/**the received ARP message must be from a neighbor which is not currently known to host*/

			if (found==0)//new node found or empty (NULL) cache then add or create cache
			{
				/**If the ARP module is a valid sender and contains information about a new neighbor add it to cache
				 * Store the new information as a new node and which will point to the current
				 * list of neighbors. The cache header point towards this new node
				 * Thus new node is added at the top of the list
				 * */
				struct node *new_host = (struct node *) malloc(sizeof(struct node));
				new_host->IP_addrs = pckt_ARP->sender_IP_addrs;
				new_host->MAC_addrs = pckt_ARP->sender_MAC_addrs;
				new_host->next = ptr_cacheHeader;
				ptr_cacheHeader = new_host;
			}
		}
	}
}


/**
 * @brief this function searches for a MAC address from a list of addresses of neighbors
 * where each neighbor has an IP and a MAC address
 * @param IP_addrs is the searched address
 * @param ptr_list_neighbors points to the first element of the list of neighbors
 */
uint64_t search_MAC_addrs(uint32_t IP_addrs, struct node *ptr_list_neighbors)
{
	uint64_t MAC_addrs = NULLADDRESS;
	struct node *ptr_elementInList = ptr_list_neighbors;

	while (ptr_elementInList!=NULL)
	{
		if (IP_addrs == ptr_elementInList->IP_addrs)
			MAC_addrs = ptr_elementInList->MAC_addrs;

		ptr_elementInList = ptr_elementInList->next;

	}
	return MAC_addrs;
}

/**
 * @brief converts 6-byte MAC address (stored as unsigned 64-bit int)
 * into a representable 6-byte char array
 * @param int_addrs is the address in unsigned int 64
 * @param *char_addrs points to the character array which will store the converted address
 *  */
void MAC_addrs_conversion(uint64_t int_addrs, unsigned char *char_addrs){
	/**register shifting is used to extract individual bytes in the code below*/
	char_addrs[5] = (unsigned char) ((int_addrs & (0x00000000000000FF)));//least sig.
	char_addrs[4] = (unsigned char) ((int_addrs & (0x000000000000FF00))>>8);
	char_addrs[3] =  (unsigned char) ((int_addrs & (0x0000000000FF0000))>>16);
	char_addrs[2] = (unsigned char) ((int_addrs & (0x00000000FF000000))>>24);
	char_addrs[1] = (unsigned char) ((int_addrs & (0x00000000FF00000000))>>32);
	char_addrs[0] = (unsigned char) ((int_addrs & (0x00FF0000000000))>>40);//most sig.
}

/**
 * @brief converts 4-byte IP address (stored as unsigned 32-bit int)
 * into a representable 4-byte char array
 * @param int_addrs is the address in unsigned int 32
 * @param *char_addrs points to the character array which will store the converted address
 *  */
void IP_addrs_conversion(uint32_t int_addrs, unsigned char *char_addrs){
	/**register shifting is used to extract individual bytes in the code below*/
	char_addrs[3] = (unsigned char) ((int_addrs & (0x000000FF)));//least significant
	char_addrs[2] = (unsigned char) ((int_addrs & (0x0000FF00))>>8);
	char_addrs[1] =  (unsigned char) ((int_addrs & (0x00FF0000))>>16);
	char_addrs[0] = (unsigned char) ((int_addrs & (0xFF000000))>>24);//most significant
}


/**
 * @brief converts an arp header into a representable format (endien/address format)
 * immediately receiving it after or before sending it outside the arp module
 * @param pckt_hdr points to the arp header whose contents will be modified
 */
void host_to_net(struct arp_hdr *pckt_hdr){

	pckt_hdr->protocol_type = htons(pckt_hdr->protocol_type);
	pckt_hdr->hardware_type = htons(pckt_hdr->hardware_type);
	pckt_hdr->operation = htons(pckt_hdr->operation);
}

/**
 * @brief simply checks whether a received ARP message contains valid information or not
 * @param pckt_arp points to the ARP message struct
 */
int check_valid_arp(struct ARP_message *pckt){

	if ((pckt!=NULL)&& (pckt->hardware_type==HWDTYPE)&&(pckt->operation==ARPREQUESTOP||pckt->operation==ARPREPLYOP)
			&&(pckt->hardware_addrs_length==HDWADDRSLEN)&&(pckt->protocol_addrs_length==PROTOCOLADDRSLEN)
			&&(pckt->protocol_type==PROTOCOLTYPE) && (pckt->sender_MAC_addrs!=NULLADDRESS)
			&&(pckt->sender_IP_addrs!=NULLADDRESS)&&(pckt->target_IP_addrs!=NULLADDRESS))
		return 1;
	else
		return 0;
}

/**
 * @brief converts an internal ARP message into a proper ARP header to be sent outside the module
 * @param ptr_msg is the pointer to the internal ARP message
 * @param ptr_hdr is the pointer to the ARP header which will be communicated outside
 *   */
void arp_msg_to_hdr(struct ARP_message *ptr_msg, struct arp_hdr *ptr_hdr){

	ptr_hdr->hardware_type = ptr_msg->hardware_type;
	ptr_hdr->protocol_type = ptr_msg->protocol_type;;
	ptr_hdr->hardware_addrs_length = ptr_msg->hardware_addrs_length;
	ptr_hdr->protocol_addrs_length = ptr_msg->protocol_addrs_length;
	ptr_hdr->operation = ptr_msg->operation;
	MAC_addrs_conversion(ptr_msg->sender_MAC_addrs, ptr_hdr->sender_MAC_addrs);
	IP_addrs_conversion(ptr_msg->sender_IP_addrs, ptr_hdr->sender_IP_addrs);
	MAC_addrs_conversion(ptr_msg->target_MAC_addrs, ptr_hdr->target_MAC_addrs);
	IP_addrs_conversion(ptr_msg->target_IP_addrs, ptr_hdr->target_IP_addrs);
}

/**
 * @brief converts an external ARP header into an internal ARP message
 * @param ptr_hdr is the pointer to the ARP header
 * @param ptr_msg is the pointer to the internal ARP message
 */
void arp_hdr_to_msg(struct arp_hdr *ptr_hdr, struct ARP_message *ptr_msg){

	unsigned char *sdr_ads, *tgt_ads, *ip_sdr, *ip_dst;

	sdr_ads = ptr_hdr->sender_MAC_addrs;
	tgt_ads = ptr_hdr->target_MAC_addrs;
	ip_sdr = ptr_hdr->sender_IP_addrs;
	ip_dst = ptr_hdr->target_IP_addrs;

	ptr_msg->hardware_type = ptr_hdr->hardware_type;
	ptr_msg->protocol_type = ptr_hdr->protocol_type;;
	ptr_msg->hardware_addrs_length = ptr_hdr->hardware_addrs_length;
	ptr_msg->protocol_addrs_length = ptr_hdr->protocol_addrs_length;
	ptr_msg->operation = ptr_hdr->operation;
	ptr_msg->sender_MAC_addrs = gen_MAC_addrs(sdr_ads[0], sdr_ads[1], sdr_ads[2], sdr_ads[3], sdr_ads[4], sdr_ads[5]);
	ptr_msg->sender_IP_addrs = gen_IP_addrs((ip_sdr[0]), (ip_sdr[1]), (ip_sdr[2]), (ip_sdr[3]));
	ptr_msg->target_MAC_addrs = gen_MAC_addrs(tgt_ads[0], tgt_ads[1], tgt_ads[2], tgt_ads[3], tgt_ads[4], tgt_ads[5]);
	ptr_msg->target_IP_addrs = gen_IP_addrs(ip_dst[0], ip_dst[1], ip_dst[2], ip_dst[3]);
}

