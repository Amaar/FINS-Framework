/**@file test_arp.c
 *@brief this files contains all the functions to test an ARP module SINGLE INTERFACE
 *@author Syed Amaar Ahmad
 *@date  September 27, 2010
 */
#include "finstypes.h"
#include "test_arp.h"//this header file already contains #include "arp.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "finsdebug.h"
#include "metadata.h"

#define DEBUG

struct node *ptr_neighbor_list;
int num_hosts; /*<the number of neighbors to be generated*/

uint64_t host_MAC_addrs;/**<MAC address of current interface; sent to the arp module*/
uint32_t host_IP_addrs;/**<IP address of current interface; sent to the arp module*/

//unsigned char *IP_addrs; /**<This contains an IP address (from heap) which is used by the paramValue of a FINS control frame*/
//struct arp_hdr *arp_net;

struct node *ptr_neighbor_list; /**<pointer the first element of a list of 'neighbors'*/
FILE *ptr_file; /**<file pointer to the file which contains a list of neighbors*/

/**
 * @brief this function is used to read an IP address from a user
 */
uint32_t read_IP_addrs()
{
	uint32_t IP_target_addrs;

	uint8_t b1, b2, b3, b4; /**<octets representing the IP address*/
	char IP_string[20];
	char *a1, *a2, *a3, *a4; /**<temporary variables to store parsed IP octet strings*/
	int test_int;

	PRINT_DEBUG("Enter IP address (e.g. 234.0.17.8) of host\n");
	scanf("%s",IP_string);

	test_int = atoi(IP_string);

	a1 = strtok(IP_string, ".");
	b1 = atoi(a1);
	a2 = strtok( NULL, "." );
	b2 = atoi(a2);
	a3 = strtok( NULL, "." );
	b3 = atoi(a3);
	a4 = strtok( NULL, "." );
	b4 = atoi(a4);

	IP_target_addrs = gen_IP_addrs(b1,b2,b3,b4);

	if (IP_target_addrs == 0)
		IP_target_addrs = NULLADDRESS;

	return IP_target_addrs;
}

/** @brief generates a binary file with artificially generated hosts (with random IP and
 * MAC addresses within a network
 * @param fileName is the name of the file where these addresses are stored
 */
void gen_neighbor_list(char *fileName)
{
	//	int num_hosts; /**<number of neighbors to be generated*/
	int i; /**< This is a variable for iterating through number of records <recordsNum>*/

	/**< The following variables are for storing octet values*/
	uint8_t IPa, IPb, IPc, IPd, MACa, MACb, MACc, MACd, MACe, MACf;

	struct node record;

	ptr_file =fopen(fileName,"w");
	if (!ptr_file)
	{	PRINT_DEBUG("Unable to open file!");
	exit (0);
	}

	PRINT_DEBUG("How many neighbors in a network to create?\n");
	fflush(stdout);
	scanf("%d", &num_hosts);

	srand ( (unsigned)time (0) );

	for (i=0;i<num_hosts;i++)
	{
		MACa = (rand())%255;
		MACb = (rand())%255;
		MACc = (rand())%255;
		MACd = (rand())%255;
		MACe = (rand())%255;
		MACf = (rand())%255;
		IPa = (rand())%255;
		IPb = (rand())%255;
		IPc = (rand())%255;
		IPd = (rand())%255;
		record.IP_addrs = gen_IP_addrs(IPa,IPb,IPc,IPd);
		record.MAC_addrs = gen_MAC_addrs(MACa,MACb,MACc,MACd,MACe, MACf);
		fwrite(&record, sizeof(struct node), 1, ptr_file);
	}

	fclose(ptr_file);    // closes the file
}

/**@brief this function reads a list of artificially created neighbor's list of a host
 * @param fileName is the file from which a list is generated
 */
struct node* read_neighbor_list(char* fileName)
		{
	int i,j; /**<temporary variables for condition testing purposes*/

	struct node *ptr_elementInList1, *ptr_elementInList2, *new_host, ptr_list; /**<These variables are used to store
	the read struct data from the file*/

	if((ptr_file=fopen(fileName, "r")) == NULL) {
		PRINT_DEBUG("Cannot open file.\n");
		exit(0);
	}

	j = 0;
	i = 0;

	while (!feof(ptr_file) && j<num_hosts)
	{
		fread(&ptr_list,sizeof(struct node),1,ptr_file);

		new_host = (struct node *) malloc (sizeof(struct node));

		new_host->IP_addrs = ptr_list.IP_addrs;
		new_host->MAC_addrs=ptr_list.MAC_addrs;
		new_host->next = NULL;

		if (j==0)
		{ ptr_elementInList1 = new_host;
		ptr_elementInList2 = new_host;
		}
		else
		{
			ptr_elementInList2->next = new_host;
			ptr_elementInList2 = new_host;
		}

		j=j+1;
	}

	fclose(ptr_file);    // closes the file
	return ptr_elementInList1;
		}

/**
 * @brief this function reads from a binary file a list of nodes (neighbors) used in testing
 * @param fileName is the name of the binary file
 */
void init_recordsARP(char *fileName){

	ptr_file =fopen(fileName,"r");  //open file

	if (!ptr_file)  // exit if file cannot open
		exit(0);

	ptr_neighbor_list = read_neighbor_list(fileName);  /**initialize the table from the file*/

}


/**@brief this function mimics ARP request from some neighbor within a network
 * @param IP_sender_addrs is the IP address of the neighbor which has sent a 'request'
 * @param MAC_sender_addrs is the MAC address of the neighbor
 * @param request_ARP_ptr is the pointer to the ARP message struct
 */
void mimic_net_request(uint32_t IP_sender_addrs, uint64_t MAC_sender_addrs,
		struct ARP_message *request_ARP_ptr)
{
	request_ARP_ptr->hardware_type = (HWDTYPE);
	request_ARP_ptr->protocol_type = (PROTOCOLTYPE);
	request_ARP_ptr->hardware_addrs_length= HDWADDRSLEN ;
	request_ARP_ptr->protocol_addrs_length = PROTOCOLADDRSLEN;
	request_ARP_ptr->operation = (ARPREQUESTOP);
	request_ARP_ptr->sender_MAC_addrs = MAC_sender_addrs;
	request_ARP_ptr->sender_IP_addrs = IP_sender_addrs;
	request_ARP_ptr->target_MAC_addrs = 0;
	request_ARP_ptr->target_IP_addrs = host_IP_addrs;

}

/**Based on which host matches the ARP request an appropriate reply through a FINS frame is
 * created
 * @brief this function mimics a network response for an ARP request pushed via a FINS frame.
 * The purpose is to test the functionality of the code for the ARP module
 * @param request_ARP_ptr is the pointer to the ARP message struct request received by the 'nodes'
 * @param reply_ARP_ptr is the pointer to the ARP message struct reply given the appropriate node
 */
void mimic_net_reply(struct ARP_message *request_ARP_ptr, struct ARP_message *reply_ARP_ptr)
{
	struct node *ptr_elementInList;
	struct ARP_message reply_ARP;

	ptr_elementInList = ptr_neighbor_list;

	while (ptr_elementInList!=NULL )
	{
		if (ptr_elementInList->IP_addrs==request_ARP_ptr->target_IP_addrs)
		{
			reply_ARP.sender_IP_addrs = ptr_elementInList->IP_addrs;
			reply_ARP.sender_MAC_addrs = ptr_elementInList->MAC_addrs;
			reply_ARP.target_IP_addrs =request_ARP_ptr->sender_IP_addrs;
			reply_ARP.target_MAC_addrs = request_ARP_ptr->sender_MAC_addrs;
			reply_ARP.hardware_addrs_length = request_ARP_ptr->hardware_addrs_length;
			reply_ARP.hardware_type = (request_ARP_ptr->hardware_type);
			reply_ARP.operation = (ARPREPLYOP);
			reply_ARP.protocol_addrs_length=request_ARP_ptr->protocol_addrs_length;
			reply_ARP.protocol_type= (request_ARP_ptr->protocol_type);
			memcpy(reply_ARP_ptr, &reply_ARP, sizeof(struct ARP_message));
		}
		ptr_elementInList = ptr_elementInList->next;
	}
}

/**@brief this generates a Fins frame which is sent to the arp module so that the
 * (1) a host receives an ARP reply, OR (2) sends an ARP request to a network
 * @param fins_frame is the pointer to the received fins frame
 * @param task indicates whether the arp message is a request or a reply to or from network
 */
void fins_from_net(struct finsFrame *fins_frame, int task)
{
	uint32_t IP_addrs_read;
	uint64_t MAC_addrs;

	struct ARP_message *msg1 = (struct ARP_message*)malloc(sizeof(struct ARP_message));
	struct ARP_message *msg2 = (struct ARP_message*)malloc(sizeof(struct ARP_message));
	struct arp_hdr *arp_net = (struct arp_hdr*) malloc(sizeof(struct arp_hdr));

	PRINT_DEBUG("\nFins data frame which carries a request or reply ARP from a network\n");

	IP_addrs_read = read_IP_addrs();
	MAC_addrs = search_MAC_addrs(IP_addrs_read, ptr_neighbor_list);

	if (task==1){
		mimic_net_request(IP_addrs_read, MAC_addrs, msg1);
		arp_msg_to_hdr(msg1, arp_net);
		host_to_net(arp_net);
		arp_to_fins(arp_net, fins_frame);
	}
	else if (task==2){
		gen_requestARP(IP_addrs_read, msg1);
		mimic_net_reply(msg1, msg2);
		arp_msg_to_hdr(msg2, arp_net);
		host_to_net(arp_net);
		arp_to_fins(arp_net, fins_frame);
	}
	fins_frame->destinationID.id = ARPID;

	free(msg1);
	free(msg2);
}

/**@brief this function generates a fins frame from the ethernet stub
 * @param fins_frame is the pointer to the fins frame to be sent into the arp
 */
void fins_from_stub(struct finsFrame *fins_frame){

	uint32_t IP_addrs_read;

	PRINT_DEBUG("\nFins control frame from link layer stub\n");
	IP_addrs_read = read_IP_addrs();
	fins_frame->dataOrCtrl = CONTROL;
	fins_frame->destinationID.id = ARPID;
	fins_frame->ctrlFrame.opcode = READREQUEST;
	fins_frame->ctrlFrame.serialNum = 123;
	fins_frame->ctrlFrame.senderID = ETHERSTUBID;
	fins_frame->ctrlFrame.paramterID = IP_addrs_read; //IP address passed through this variable
}

/**
 * @brief for a given ARP message this function prints the IP and MAC addresses
 * of the sender and the target
 * @param pckt is the ARP request or reply which has been generated by a host
 */
void print_msgARP(struct ARP_message *pckt){

	if (pckt->operation == ARPREQUESTOP)
		printf("\nARP Message Request");
	if (pckt->operation == ARPREPLYOP)
		printf("\nARP Message Reply");

	printf("\nSender:");
	print_IP_addrs(pckt->sender_IP_addrs);
	print_MAC_addrs(pckt->sender_MAC_addrs);
	printf("\n Hardware Address Length : %u",pckt->hardware_addrs_length);
	printf("\n Hardware Type : %d",pckt->hardware_type);
	printf("\n Protocol Address Length : %u",pckt->protocol_addrs_length);
	printf("\n Protocol Type : %d",pckt->protocol_type);
	printf("\n Operation Type : %d",pckt->operation);
	printf("\nTarget:");
	print_IP_addrs(pckt->target_IP_addrs);
	print_MAC_addrs(pckt->target_MAC_addrs);

}

void print_arp_hdr(struct arp_hdr *pckt){

	int i;

	printf("\n\nPrinting of an external format arp message");
	printf("\nSender hardware (MAC) address = ");
	for (i=0;i<HDWADDRSLEN;i++)
		printf("%x:", pckt->sender_MAC_addrs[i]);
	printf("\nSender IP address = ");
	for (i=0;i<PROTOCOLADDRSLEN;i++)
		printf("%d.", pckt->sender_IP_addrs[i]);
	printf("\nTarget hardware (MAC) address= ");
	for (i=0;i<HDWADDRSLEN;i++)
		printf("%x:", pckt->target_MAC_addrs[i]);
	printf("\nTarget IP address = ");
	for (i=0;i<PROTOCOLADDRSLEN;i++)
		printf("%d.", pckt->target_IP_addrs[i]);
	printf("\nHardware type: %d", pckt->hardware_type);
	printf("\nProtocol type: %d", pckt->protocol_type);
	printf("\nHardware length: %d", pckt->hardware_addrs_length);
	printf("\nHardware length: %d", pckt->protocol_addrs_length);
	printf("\nOperation: %d\n\n", pckt->operation);
}

/**
 * @brief this function prints the contents of a cache for each of the interfaces
 * ptr_cacheHeader points to the first element/header of the cache
 */
void print_cache(struct node *ptr_elementInList){

	print_neighbors(ptr_elementInList);
	printf("\n\n");
}

/**
 * @brief this function prints the list of addresses of a host's neighbors
 * (useful in testing/mimicing network response)
 * @param ptr_neighbors points to the first element of the list of 'neighbors'
 */
void print_neighbors(struct node *ptr_list_neighbors){

	struct node *ptr_elementInList;

	ptr_elementInList = ptr_list_neighbors;
	printf("\nList of addresses of neighbors:\n");

	while (ptr_elementInList!=NULL){
		print_IP_addrs(ptr_elementInList->IP_addrs);
		print_MAC_addrs(ptr_elementInList->MAC_addrs);
		printf("\n");
		ptr_elementInList = ptr_elementInList->next;
	}
}

/**
 * @brief this function converts an ARP message into a FINS frame
 * @param reply_ARP is a pointer to an ARP message struct which is type cast as a PDU
 * of a data FINS frame
 */
void arp_to_fins(struct arp_hdr *pckt_arp, struct finsFrame *pckt_fins)
{
	pckt_fins->destinationID.id = (unsigned char) ETHERSTUBID;
	pckt_fins->dataOrCtrl = DATA;
	pckt_fins->dataFrame.pdu = (unsigned char *)(pckt_arp);
	pckt_fins->dataFrame.directionFlag= DOWN;
	pckt_fins->dataFrame.pduLength = sizeof(struct arp_hdr);
}

/**
 * @brief this function converts a FINS frame into an ARP message
 * @param pckt_fins is a FINS data frame whose pdu contains the address of an ARP message struct
 * @param reply_ARP is an 'empty' ARP message which will be be filled by the contents pointed to by the FINS frame's pdu
 */
void fins_to_arp(struct finsFrame *pckt_fins, struct arp_hdr *pckt_arp){

	memcpy(pckt_arp,  pckt_fins->dataFrame.pdu, pckt_fins->dataFrame.pduLength);
}

/**
 * @brief this function prints IP address in a human readable format
 * @param IP_addrs is the uint32 address
 */
void print_IP_addrs(uint32_t IP_addrs)
{
	uint8_t a, b, c, d; /**<a,b,c,d are octets of an IP address (e.g. a.b.c.d)*/

	a = IP_addrs/(16777216);
	b = (IP_addrs - a*16777216)/65536;
	c = (IP_addrs - a*16777216 - b*65536)/(256);
	d = (IP_addrs - a*16777216 - b*(256*256) -c*256);
	printf("IP address = %u.%u.%u.%u ",a,b,c,d);

}

/**
 * @brief this function prints a MAC address in a readable format
 * @param IP_addrs is the uint64 address (although a 48-byte address is used in practice
 */
void print_MAC_addrs(uint64_t MAC_intg_addrs)
{	printf("MAC address = %llx", MAC_intg_addrs);}

void arp_test_harness()
{
	struct finsFrame fins_frame, *fins_ptr;
	int task;

	fins_ptr = &fins_frame;

	init_arp_intface(host_MAC_addrs, host_IP_addrs);

	task = 1;
	while (task!=0){

		PRINT_DEBUG("\nReceive from network a request arp `1' or reply arp `2' or\n generate request to network `3', `0' to exit\n");
		scanf("%d", &task);

		if ((task==1)||(task==2))
			fins_from_net(fins_ptr, task);
		else if (task==3)
			fins_from_stub(fins_ptr);

		if (task==1 ||task==2 ||task==3)
		arp_in(fins_ptr);
	}

	term_arp_intface();
}

int main(int argc, char *argv[])
{
	uint64_t MACADDRESS = 9890190479;/**<MAC address of host; sent to the arp module*/
	uint32_t IPADDRESS = 672121;/**<IP address of host; sent to the arp module*/

	host_MAC_addrs = MACADDRESS;
	host_IP_addrs = IPADDRESS;

	gen_neighbor_list(argv[1]);

	init_recordsARP(argv[1]);
	print_neighbors(ptr_neighbor_list);

	arp_test_harness(); //test functionality of ARP module

	return 0;
}
