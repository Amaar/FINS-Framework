/**@file test_arp.c
 *@brief this files contains all the functions to test an ARP module SINGLE INTERFACE
 *@author Syed Amaar Ahmad
 *@date  September 27, 2010
 */
#include "finstypes.h"
#include "arp.h"
#include "test_arp.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int num_hosts;
struct node *ptr_neighbor_list; /**<pointer the first element of a list of 'neighbors'*/
FILE *ptr_file; /**<file pointer to the file which contains a list of neighbors*/

/**@brief this function mimics ARP request from some neighbor within a network
 * @param IP_sender_addrs is the IP address of the neighbor which has sent a 'request'
 * @param MAC_sender_addrs is the MAC address of the neighbor
 */
struct ARP_message mimic_net_request(uint32_t IP_sender_addrs, uint64_t MAC_sender_addrs)
{
	struct ARP_message request_ARP;
	extern uint32_t interface_IP_addrs;

	request_ARP.hardware_type = HWDTYPE;
	request_ARP.protocol_type = PROTOCOLTYPE;
	request_ARP.hardware_addrs_length= HDWADDRSLEN ;
	request_ARP.protocol_addrs_length = PROTOCOLADDRSLEN;
	request_ARP.operation = ARPREQUESTOP;
	request_ARP.sender_MAC_addrs = MAC_sender_addrs;
	request_ARP.sender_IP_addrs = IP_sender_addrs;
	request_ARP.target_MAC_addrs = 0;
	request_ARP.target_IP_addrs = interface_IP_addrs;

	return request_ARP;
}

/**Based on which host matches the ARP request an appropriate reply through a FINS frame is
 * created
 * @brief this function mimics a network response for an ARP request pushed via a FINS frame.
 * The purpose is to test the functionality of the code for the ARP module
 * @param request_fins is FINS data frame which hosts within a network will receive
 * from the Switch
 */
struct finsFrame mimic_net_reply(struct finsFrame request_fins)
{
	struct node *ptr_elementInList;
	struct ARP_message request_ARP, reply_ARP, *temp_ARP;
	struct finsFrame reply_fins;

	ptr_elementInList = ptr_neighbor_list;

	temp_ARP = (struct ARP_message*) request_fins.dataFrame.pdu;
	request_ARP=*temp_ARP;

	ptr_elementInList = ptr_neighbor_list;

	request_ARP = fins_to_arp(request_fins);

	print_msgARP(request_ARP);

	/**Default entries*/
	reply_ARP.sender_IP_addrs = NULLADDRESS;
	reply_ARP.sender_MAC_addrs = NULLADDRESS;
	reply_ARP.target_IP_addrs = NULLADDRESS;
	reply_ARP.target_MAC_addrs = NULLADDRESS;
	reply_ARP.operation = NULLADDRESS;

	while (ptr_elementInList!=NULL )
	{

		if (ptr_elementInList->IP_addrs==request_ARP.target_IP_addrs)
		{
			reply_ARP.sender_IP_addrs = ptr_elementInList->IP_addrs;
			reply_ARP.sender_MAC_addrs = ptr_elementInList->MAC_addrs;
			reply_ARP.target_IP_addrs =request_ARP.sender_IP_addrs;
			reply_ARP.target_MAC_addrs = request_ARP.sender_MAC_addrs;
			reply_ARP.hardware_addrs_length = request_ARP.hardware_addrs_length;
			reply_ARP.hardware_type = request_ARP.hardware_type;
			reply_ARP.operation = ARPREPLYOP;
			reply_ARP.protocol_addrs_length=request_ARP.protocol_addrs_length;
			reply_ARP.protocol_type=request_ARP.protocol_type;

		}

		ptr_elementInList = ptr_elementInList->next;
	}
	temp_ARP=&reply_ARP;
	print_msgARP(reply_ARP);

	reply_fins =  arp_to_fins(&reply_ARP);

	return reply_fins;
}

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

	printf("Segmentation fault may result if address improperly entered\n"
			"Enter IP address (e.g. 234.0.17.8) of target; (0.0.0.0) to exit function:\n");
	fflush(stdout);
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
	{	printf("Unable to open file!");
		exit (0);
	}

	printf("How many neighbors in a network to create?\n");
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
		printf("Cannot open file.\n");
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
		new_host->co_intface = NULL;

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

/**@brief this function tests a set of functions which are used when
 * (1) a host sends an ARP request, (2) the request is converted to and from a FINS frame
 * (3) a cache is updated one time based on the request
 * @param fileName is the file from which the list of neighbors is drawn
 */
void sender_test(char *fileName)
{
	extern struct node *ptr_cacheHeader;

	struct finsFrame request_fins, reply_fins;
	struct ARP_message request_ARP, reply_ARP;
	struct node *ptr_elementInList;

	uint32_t IP_addrs_read = 1;

	printf("\nTest of sending ARP messages\n");
	fflush(stdout);

	ptr_cacheHeader = init_intface();
	print_cache();

	gen_neighbor_list(fileName);
	init_recordsARP(fileName);
	print_cache();
	print_neighbors(ptr_neighbor_list);

	printf("\nChoose Sending Node Test (ARP module sends request to network for reply)\n'0' to exit\n");

	while (IP_addrs_read!=0)
	{
		IP_addrs_read = read_IP_addrs();

		request_ARP = gen_requestARP(IP_addrs_read);

		request_fins = arp_to_fins(&request_ARP);

		reply_fins = mimic_net_reply(request_fins);

		reply_ARP = fins_to_arp(reply_fins);

		update_cache(reply_ARP);

		print_cache(ptr_cacheHeader);
	}


	term_intface();
}

/**@brief this function tests a set of functions which are used when
 * (1) a host receives an ARP request, and (2) keeps updating its cache based on
 * these ARP requests
 * @param fileName is the file from which the list of neighbors is drawn
 */
void receiver_test(char *fileName)
{
	struct ARP_message reply_ARP;

	extern struct node *ptr_cacheHeader;
	struct node *ptr_elementInList;

	uint32_t IP_sender_addrs = 1;
	uint64_t MAC_sender_addrs;


	printf("\nTest of receiving ARP messages\n");
	fflush(stdout);

	gen_neighbor_list(fileName);
	init_recordsARP(fileName);
	print_neighbors(ptr_neighbor_list);

	ptr_cacheHeader = init_intface();

	printf("\nFrom the above neighbor list choose which node (IP) should generate an ARP message?\n'0' to exit\n");
	fflush(stdout);

	while (IP_sender_addrs!=NULLADDRESS)
	{
		IP_sender_addrs = read_IP_addrs();

		MAC_sender_addrs = search_MAC_addrs(IP_sender_addrs, ptr_neighbor_list);
		reply_ARP = mimic_net_request(IP_sender_addrs, MAC_sender_addrs);
		print_msgARP(reply_ARP);
		update_cache(reply_ARP);
		print_cache();
	}

	term_intface();

}


