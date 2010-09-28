/**@file arp.c
 *@brief this files contains all relevant functions to execute an ARP module,
 *@brief IP and MAC address of the host is provided by the main
 *@author Syed Amaar Ahmad
 *@date  September 27, 2010
 */

#include <stdio.h>
#include <stdlib.h>
#include "finstypes.h"
#include "test_arp.h"
#include "arp.h"

struct node *ptr_cacheHeader; /**< points to the first element of the dynamic ARP cache*/

/**
 * An address like a.b.c.d (e.g. 5.45.0.07 where a= 5, b=45, c=0,d=7) is converted an integer
 * @brief this function takes a user defined address and produces a uint32 address
 * @param a an octet
 * @param b an octet
 * @param c an octet
 * @param d an octet
 */
uint32_t gen_IP_addrs(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{	return (16777216ul*(a) + (65536ul*(b)) + (256ul*(c)) + (d));
}

/**
 * An address like a:b:c:d:e:f is converted into an 64-byte unsigned integer
 * @brief this function takes a user provided MAC address as a set of octets and produces a uint64 address
 * @param a an octet
 * @param b an octet
 * @param c an octet
 * @param d an octet
 * @param e an octetvoid init_recordsARP(char *fileName);*/

uint64_t gen_MAC_addrs(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
{return (109951162800ull*(a)+ 4294967296ull*(b) + 16777216ull*(c) + 65536ull*(d) + (256ull*(e)) + (f));
}

/**
 * @brief this function prints IP address in a human readable format
 * @param IP_addrs is the uint32 address
 */
void print_IP_addrs(uint32_t IP_addrs)
{
	uint8_t a, b, c, d; /**<a,b,c,d are octets of an IP address (e.g. a.b.c.d)*/

	a = IP_addrs/(256*256*256);
	b = (IP_addrs - a*(256*256*256))/(256*256);
	c = (IP_addrs - a*(256*256*256) - b*(256*256))/(256);
	d = (IP_addrs - a*(256*256*256) - b*(256*256) -c*256);
	printf("\nIP address = %u.%u.%u.%u ",a,b,c,d);
	fflush(stdout);

}

/**
 * @brief this function prints a MAC address in a human readable format
 * @param IP_addrs is the uint64 address (although a 48-byte address is used in practice
 */
void print_MAC_addrs(uint64_t MAC_addrs)
{
	uint8_t a, b, c, d, e, f; /**<a,b,d,c,d,e,f are octets of a MAC address*/

	a = MAC_addrs/(109951162800);
	b = (MAC_addrs - a*(109951162800))/(4294967296);
	c = (MAC_addrs - a*(109951162800) - b*(4294967296))/(16777216);
	d = (MAC_addrs - a*(109951162800) - b*(4294967296) -c*(16777216))/(65536);
	e = (MAC_addrs - a*(109951162800) - b*(4294967296) -c*(16777216) - d*(65536)	)/(256);
	f = (MAC_addrs - a*(109951162800) - b*(4294967296) -c*(16777216) - d*(65536)	-e*256);

	printf("MAC address = %x:%x:%x:%x:%x:%x\n",a,b,c,d,e,f);
	fflush(stdout);

}

/**
 * @brief this function produces an ARP request for a host whose IP address is known
 * @param IP_address_target is the uint32 address of the target host
 */
struct ARP_message gen_requestARP(uint32_t IP_address_target)
{
	struct ARP_message request_ARP;

	extern uint64_t interface_MAC_addrs;
	extern uint32_t interface_IP_addrs;

	request_ARP.hardware_type = HWDTYPE;
	request_ARP.protocol_type = PROTOCOLTYPE;
	request_ARP.hardware_addrs_length= HDWADDRSLEN ;
	request_ARP.protocol_addrs_length = PROTOCOLADDRSLEN;
	request_ARP.operation = ARPREQUESTOP;
	request_ARP.sender_MAC_addrs = interface_MAC_addrs;
	request_ARP.sender_IP_addrs = interface_IP_addrs;
	request_ARP.target_MAC_addrs = 0;
	request_ARP.target_IP_addrs = IP_address_target;

	return request_ARP;
}

/**
 * @brief this function produces an ARP reply for the host which has already sent
 * a request for a MAC address
 * @param request_ARP is the ARP request from the host
 */
struct ARP_message gen_replyARP(struct ARP_message request_ARP)
{
	struct ARP_message reply_ARP;

	extern uint64_t interface_MAC_addrs;
	extern uint32_t interface_IP_addrs;

	reply_ARP.hardware_type = HWDTYPE;
	reply_ARP.protocol_type = PROTOCOLTYPE;
	reply_ARP.hardware_addrs_length= HDWADDRSLEN ;
	reply_ARP.protocol_addrs_length = PROTOCOLADDRSLEN;
	reply_ARP.operation = ARPREPLYOP;
	reply_ARP.sender_MAC_addrs = interface_MAC_addrs;
	reply_ARP.sender_IP_addrs = interface_IP_addrs;
	reply_ARP.target_MAC_addrs = request_ARP.sender_MAC_addrs;
	reply_ARP.target_IP_addrs = request_ARP.sender_IP_addrs;

	return reply_ARP;
}

/**
 * @brief this function searches a list or cache for to check if it contains a
 * particular MAC address, returns 1 if found else returns 0
 * @param ptr_intface_cache is the pointer to the first element of the cache
 * @param MAC_addrs is the searched MAC address from the cache
 */
int search_list(struct node *ptr_firstElementOfList, uint64_t MAC_addrs)
{
	struct node * ptr_elementInList;
	int found = 0;

	ptr_elementInList = ptr_firstElementOfList;

	while (ptr_elementInList!=NULL)
	{

		if (ptr_elementInList->MAC_addrs == MAC_addrs )
			found = 1;

		ptr_elementInList = ptr_elementInList->next;
	}

	return found;
}

/** * ptr_cacheHeader is the pointer to the first element of the cache
 * @brief this function updates a cache based on an ARP reply
 * @param pckt_ARP is an ARP message (either reply or request) from some host within the neighborhood
*/
void update_cache(struct ARP_message pckt_ARP)
{
	uint32_t IP_addrs_dest;
	int found = 0;
	struct node *ptr_interfaceHeader, *ptr_elementInList;/**these variables are used to traverse the linked list structure of cache*/

	/**cache header is a node which has information about the interface of
	 * the host itself. It links via 'next' pointer to the node containing addresses of
	 * a neighbor. Each subsequent neighbor is linked via this pointer
	 * */
	ptr_interfaceHeader = ptr_cacheHeader;

	while (ptr_interfaceHeader!=NULL)
	{
		/**point pointer to the list/cache header*/
		ptr_elementInList = ptr_interfaceHeader;
		/**returns 1 if found, else 0*/
		found = search_list(ptr_elementInList, pckt_ARP.sender_MAC_addrs);
		/**check for all of host's interfaces though not used in current version*/
		ptr_interfaceHeader = ptr_interfaceHeader->co_intface;
	}

	/**the received ARP message must be from a neighbor which is not currently known to host*/
	ptr_interfaceHeader = ptr_cacheHeader;

	while (ptr_interfaceHeader!=NULL && found==0)
	{
		if (pckt_ARP.target_IP_addrs!=NULLADDRESS && pckt_ARP.sender_MAC_addrs!=NULLADDRESS && pckt_ARP.sender_IP_addrs!=NULLADDRESS)
		{
			/**If the ARP module is a valid sender and contains information about a new neighbor add it to cache
			 * Store the new information as a new node and which will point to the current
			 * list of neighbors. The cache header point towards this new node
			 * Thus new node is added at the top of the list
			 * */

			struct node *new_host = malloc(sizeof(struct node));
			new_host->next = NULL;
			new_host->IP_addrs = pckt_ARP.sender_IP_addrs;
			new_host->MAC_addrs = pckt_ARP.sender_MAC_addrs;
			new_host->next = ptr_interfaceHeader->next;
			ptr_interfaceHeader->next = new_host;
		}
		ptr_interfaceHeader = ptr_interfaceHeader->co_intface;
	}

}

/**
 * @brief for a given ARP message this function prints the IP and MAC addresses
 * of the sender and the target
 * @param pckt is the ARP request or reply which has been generated by a host
 */
void print_msgARP(struct ARP_message pckt){

	if (pckt.operation == ARPREQUESTOP)
		printf("\nARP Message Request");

	else if (pckt.operation == ARPREPLYOP)
		printf("\nARP Message Reply");

	printf("\nSender:");
	fflush(stdout);

	print_IP_addrs(pckt.sender_IP_addrs);
	print_MAC_addrs(pckt.sender_MAC_addrs);

	printf("\nTarget:");
	fflush(stdout);

	print_IP_addrs(pckt.target_IP_addrs);
	print_MAC_addrs(pckt.target_MAC_addrs);

}

/**
 * @brief this function prints the contents of a cache for each of the interfaces
 * ptr_cacheHeader points to the first element/header of the cache
 */
void print_cache(){

	struct node *ptr_interfaceHeader, *ptr_elementInList;

	ptr_elementInList = ptr_cacheHeader;
	ptr_interfaceHeader = ptr_cacheHeader;

	while (ptr_interfaceHeader!=NULL){

		printf("\nHost Interface IP Address");
		print_IP_addrs(ptr_interfaceHeader->IP_addrs);
		//fflush(stdout);
		ptr_elementInList = ptr_elementInList->next; //move the pointer to the stored node
		print_neighbors(ptr_elementInList);
		ptr_interfaceHeader = ptr_interfaceHeader->co_intface;
		ptr_elementInList = ptr_interfaceHeader;
	}

}

/**
 * @brief this function prints the list of addresses of a host's neighbors
 * (useful in testing/mimicing network response)
 * @param ptr_neighbors points to the first element of the list of 'neighbors'
 */
void print_neighbors(struct node *ptr_list_neighbors){

	struct node *ptr_elementInList;

	ptr_elementInList = ptr_list_neighbors;

	printf("\n\nList of addresses of neighbors\n");
	fflush(stdout);

	while (ptr_elementInList!=NULL){

		print_IP_addrs(ptr_elementInList->IP_addrs);
//		printf("\nNumerical IP Address %u\n", ptr_elementInList->IP_addrs);
		print_MAC_addrs(ptr_elementInList->MAC_addrs);
	//	printf("Numerical MAC Address %llu\n", ptr_elementInList->MAC_addrs);
	//	fflush(stdout);
		ptr_elementInList = ptr_elementInList->next;
	}

}

/**
 * @brief this function searches for a MAC address from a list of addresses of neighbors
 * where each neighbor has an IP and a MAC address
 * @param IP_addrs is the searched address
 * @param list points to the first element of the list of neighbors
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
 * @brief this function converts an ARP message into a FINS frame
 * @param reply_ARP is a pointer to an ARP message struct which is type cast as a PDU
 * of a data FINS frame
 */
struct finsFrame arp_to_fins(struct ARP_message *pckt_ARP)
{
	struct finsFrame reply_frame;

	reply_frame.dataOrCtrl = DATA;
	reply_frame.dataFrame.pdu = (unsigned char *) pckt_ARP;

	if (pckt_ARP->operation == ARPREQUESTOP) //request
	{
		reply_frame.destinationID.id = (unsigned char) WIFISTUBID; //to be sent to the ethernet
		reply_frame.dataFrame.directionFlag= DOWN;
		reply_frame.dataFrame.pduLength = ARPMSGLENGTH;
	}

	else if (pckt_ARP->operation == ARPREPLYOP) //reply
	{
		reply_frame.dataOrCtrl = DATA;
		reply_frame.destinationID.id = (unsigned char) ARPID; //to be sent to the ethernet
		reply_frame.dataFrame.directionFlag= UP;

	}
	else
	{
		printf("\nError: Invalid address resolution protocol operation\nOR no neighbor with this address\n");		//exit(0);
	}


	return reply_frame;
}

/**
 * @brief this function converts a FINS frame into an ARP message
 * @param pckt_fins is a FINS data frame whose pdu will be type cast into an ARP message
 */
struct ARP_message fins_to_arp(struct finsFrame pckt_fins){

	struct ARP_message reply_ARP, *temp_msg;

	temp_msg = (struct ARP_message*) pckt_fins.dataFrame.pdu;
	reply_ARP = *temp_msg;
	return reply_ARP;
}

/**
 * @brief this function initializes a cache for the host's interface.
 * @brief Note that an interface is the first element/header of
 * a linked list of neighbors' which are represented as nodes and the
 * 'co-intface' pointer is used to link multiple interfaces (not used in current version)
 * each neighbor is linked to the next neighbor via the 'next' pointer
 */
struct node* init_intface()
{
    int i;
	struct node *ptr_elementInList1, *ptr_elementInList2; /**<temporary variables*/

	extern INTERFACECOUNT;	/**<indicates the number of interfaces of the host (e.g. 1)*/
	extern uint32_t IP_interface_set[1];/**<stores the IP addresses of all interfaces*/
	extern uint64_t MAC_interface_set[1];/**<stores the MAC addresses of all interfaces*/

	printf("\nInitializing ARP cache\n");
	fflush(stdout);

	for (i=0;i<INTERFACECOUNT;i++){

		struct node *intface = malloc(sizeof(struct node));

		intface->IP_addrs = IP_interface_set[i];
		intface->MAC_addrs = MAC_interface_set[i];
		intface->co_intface=NULL;
		intface->next = NULL;


		if (i==0){
			ptr_elementInList1 = intface;
			ptr_elementInList2 = intface;
		}
		else{
			ptr_elementInList2->co_intface = intface;
			ptr_elementInList2 = intface;

		}
	}
	return ptr_elementInList1;
}

/**
 * @brief this function liberates all memory allocated to store and represent the cache
 * for the ARP module */
void term_intface()
{
struct node *ptr_elementInList1, *ptr_elementInList2, *ptr_elementInList3;

	ptr_elementInList1 = ptr_cacheHeader;
	ptr_elementInList2 = ptr_cacheHeader;
	ptr_elementInList2 = ptr_cacheHeader;

	printf("\nFreeing memory of ARP cache\n");
	fflush(stdout);

	while (ptr_elementInList1!=NULL){

		ptr_elementInList2 = ptr_elementInList1->next;
		ptr_elementInList3 = ptr_elementInList2;

		while (ptr_elementInList2!=NULL){

			ptr_elementInList3 = ptr_elementInList2;
			ptr_elementInList2 = ptr_elementInList2->next;

			free(ptr_elementInList3);

		}

		ptr_elementInList2 = ptr_elementInList1->co_intface;
		free(ptr_elementInList1);
		ptr_elementInList1 = ptr_elementInList2;

	}

}

