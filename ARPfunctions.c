#include <stdio.h>
#include <stdlib.h>

#include "finstypes.h"
//#include "ARP.h"
#include "ARPfunctions.h"


struct neighbor *ptrTable; /**<this variable points to the first element
 * of any temporary linked list of records*/

/**@brief This function initializes the file
 * @ fileName is the name of the file
 */


uint32_t genIPaddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{	return (16777216ul*(a) + (65536ul*(b)) + (256ul*(c)) + (d));
}

uint64_t genMACaddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
{return (109951162800ull*(a)+ 4294967296ull*(b) + 16777216ull*(c) + 65536ull*(d) + (256ull*(e)) + (f));
}

void printIPaddress(uint32_t ip)
{
	uint8_t a, b, c, d;

	a = ip/(256*256*256);
	b = (ip - a*(256*256*256))/(256*256);
	c = (ip - a*(256*256*256) - b*(256*256))/(256);
	d = (ip - a*(256*256*256) - b*(256*256) -c*256);
	printf("\nIP address = %u.%u.%u.%u\n",a,b,c,d);
	fflush(stdout);

}

void printMACaddress(uint64_t mac)
{
	uint8_t a, b, c, d, e, f;


	a = mac/(109951162800);

	//lldiv_t a;//result;
	//         a = (mac/(256*256*256*256*256));

	//printf("\nMAC address A= %llu", mac);

	b = (mac - a*(109951162800))/(4294967296);
	c = (mac - a*(109951162800) - b*(4294967296))/(16777216);
	d = (mac - a*(109951162800) - b*(4294967296) -c*(16777216))/(65536);
	e = (mac - a*(109951162800) - b*(4294967296) -c*(16777216) - d*(65536)	)/(256);
	f = (mac - a*(109951162800) - b*(4294967296) -c*(16777216) - d*(65536)	-e*256);

	printf("MAC address = %x:%x:%x:%x:%x:%x\n",a,b,c,d,e,f);
	fflush(stdout);

}

struct ARPmsg generateARPrequest(uint32_t ipTargetAddress)
{
	struct ARPmsg Request;

	extern uint64_t macAddressInterface;
	extern uint32_t ipAddressInterface;


	Request.hardwareType = 1;
	Request.protocolType = 0x0800;
	Request.hardwareAddressLength= 6;
	Request.protocolAddressLength = 4;
	Request.operation = ARPrequestOp;
	Request.senderMACaddrs = macAddressInterface;

	Request.senderIPaddrs = ipAddressInterface;

	Request.operation = 1;

	Request.targetMACaddrs = 0;

	Request.targetIPaddrs = ipTargetAddress;

	return Request;
}


struct ARPmsg generateARPreply(struct ARPmsg Request)
{

	struct ARPmsg Reply;
	extern uint64_t macAddressInterface; //this will be loaded from the test/main code side
	extern uint32_t ipAddressInterface;


	Reply.hardwareType = 1;
	Reply.protocolType = 0x0800;
	Reply.hardwareAddressLength= 6;
	Reply.protocolAddressLength = 4;
	Reply.operation = ARPreplyOp;
	Reply.senderMACaddrs = macAddressInterface;

	Reply.senderIPaddrs = ipAddressInterface;

	Reply.targetMACaddrs = Request.senderMACaddrs;

	Reply.targetIPaddrs = Request.targetIPaddrs;

	return Reply;

}



int searchNeighbor(struct neighbor *ptrCache, uint64_t MACaddress)
{

	struct neighbor * ptrTemp;
	int found = 0;

	ptrTemp = ptrCache;

	while (ptrTemp!=NULL)
	{

		if (ptrTemp->MACaddrs == MACaddress )
			found = 1;

		ptrTemp = ptrTemp->next;
	}


	return found;
}

/*
struct neighbor * updateCache(struct neighbor *ptrCache, struct ARPmsg info)
		{

	//	struct neighbor * ptrTemp;
	int found = 0;

	found = searchNeighbor(ptrCache, info.senderMACaddrs);//if the local cache has this or not


	printf("\nFOUND = %d\n", found);

	if (found==0 && info.senderIPaddrs!=NULLADDRESS)
	{
		struct neighbor *newNeighbor = malloc(sizeof(struct neighbor));
		newNeighbor->next = NULL;

		newNeighbor->IPaddrs = info.senderIPaddrs;
		newNeighbor->MACaddrs = info.senderMACaddrs;


		if (ptrCache != NULL)
			newNeighbor->next = ptrCache;

		return newNeighbor;

	}
	else
		return ptrCache;
		}

 */

struct neighbor * updateCache(struct neighbor *ptrCache, struct ARPmsg info)
		{

	//	struct neighbor * ptrTemp;
	int found = 0;

	uint32_t destinationIPinterface;


	struct neighbor *ptrTemp1, *ptrTemp2;
	ptrTemp1 = ptrCache;


	while (ptrTemp1!=NULL)
	{

		ptrTemp2 = ptrTemp1; //ptrTemp2 points to the interface Header things
		found = searchNeighbor(ptrTemp2, info.senderMACaddrs);//if the local cache has this or not

		ptrTemp1 = ptrTemp1->coHost;


	}

	//identify the interface to which connect

	destinationIPinterface = info.targetIPaddrs ; // this is the intended destination and is by default supposed to match the id

	ptrTemp1 = ptrCache;


	while (ptrTemp1!=NULL)
	{

		if (destinationIPinterface==ptrTemp1->IPaddrs && found==0 && info.senderIPaddrs!=NULLADDRESS)
		{

			printf("\n\nFOUND = %d\n", found);
			fflush(stdout);

			struct neighbor *newNeighbor = malloc(sizeof(struct neighbor));
			newNeighbor->next = NULL;

			newNeighbor->IPaddrs = info.senderIPaddrs;
			newNeighbor->MACaddrs = info.senderMACaddrs;


			newNeighbor->next = ptrTemp1->next;
			ptrTemp1->next = newNeighbor;


		}

		ptrTemp1 = ptrTemp1->coHost;
		//by default the ptrTemp1 pointer to a MAC address would only match the interface MAC address once
	}//end inner while loop

	return ptrCache;//the pointer to the first element is always passed
		}


void printARP(struct ARPmsg msg){

	//	printf("Hardware Type %u, protocol Type %u, hardware address length %u,Protocol address length %u,Operation %u\n", msg.targetIPaddrs);
	printf("\nSender MAC %llu\n Sender IP %u\n Target MAC %llu\n Target IP %u\n", msg.senderMACaddrs, msg.senderIPaddrs,msg.targetMACaddrs, msg.targetIPaddrs);
	fflush(stdout);

}

void printLocalCache(struct neighbor *nodePtr){

	struct neighbor *node1, *node2;

	node1 = nodePtr;
	node2 = nodePtr;


	while (node1!=NULL){

		printf("\nINTERFACE IP Address: %u ", node1->IPaddrs);
		node2 = node2->next; //touch the neighbor rather than itself

		while(node2!=NULL){

		printIPaddress(node2->IPaddrs);

		printf("\nNumerical IP Address %u\n", node2->IPaddrs);

		printMACaddress(node2->MACaddrs);
		printf("Numerical MAC Address %llu\n\n", node2->MACaddrs);
		fflush(stdout);

		node2=node2->next;
		}

		//	fflush(stdout);
		node1 = node1->coHost;
		node2 = node1;
	}

}

void printNeighbors(struct neighbor *nodePtr){

	struct neighbor *node;

	node = nodePtr;

	while (node!=NULL){


		printIPaddress(node->IPaddrs);

		printf("\nNumerical IP Address %u\n", node->IPaddrs);

		printMACaddress(node->MACaddrs);
		printf("Numerical MAC Address %llu\n\n", node->MACaddrs);
		fflush(stdout);


		node = node->next;
	}

}


uint64_t searchMACaddress(uint32_t IPaddress, struct neighbor *node)
{
	uint64_t macAddress = NULLADDRESS;

	struct neighbor *ptr = node;

	while (ptr!=NULL)
	{
		if (IPaddress == ptr->IPaddrs)
			macAddress = ptr->MACaddrs;

		ptr = ptr->next;

	}

	return macAddress;
}


uint32_t readIPaddress()
{

	uint32_t ipTargetAddress;
	char *ipString;

	printf("Enter IP address of target destination (234.45.76.112)\n");

	//scanf("%s", ipString);

	scanf("%u",&ipTargetAddress);

	return ipTargetAddress;

}


struct finsFrame genFinsFrame(struct ARPmsg *rcvdFrame){

	struct finsFrame sentFrame;

	sentFrame.dataOrCtrl = DATA;
	sentFrame.dataFrame.pdu = (unsigned char *) rcvdFrame ;


	//	printf("Testing operation type %d\n\n", rcvdFrame->operation);

	if (rcvdFrame->operation == ARPrequestOp) //request
	{


		sentFrame.destinationID.id = (unsigned char) WIFISTUBID; //to be sent to the ethernet
		sentFrame.dataFrame.directionFlag= DOWN;
		sentFrame.dataFrame.pduLength = 208;


	}
	else if (rcvdFrame->operation == ARPreplyOp) //reply
	{
		sentFrame.dataOrCtrl = DATA;
		sentFrame.destinationID.id = (unsigned char) IPID; //to be sent to the ethernet
		sentFrame.dataFrame.directionFlag= UP;

	}
	else
	{
		printf("\nError: Invalid address resolution protocol operation\nOR no neighbor with this address\n");
		//exit(0);
	}


	return sentFrame;
}

struct ARPmsg genARPmsg(struct finsFrame rcvdFrame){

	struct ARPmsg sentMsg, *tempMsg;
	tempMsg = (struct ARPmsg*) rcvdFrame.dataFrame.pdu;

	sentMsg = *tempMsg;

	return sentMsg;
}


void ARPInitialize(char *fileName){

	extern struct neighbor *ptrGlobal;
	extern FILE *ptrMyfile;

	ptrMyfile =fopen(fileName,"r");  //open file


	if (!ptrMyfile)  // exit if file cannot open


		exit(0);


	ptrGlobal = ReadNeighborList(fileName);  /**initialize the table from the file*/

}

/**@brief This function closes the file of records
 */

void ARPTerminate(){

	extern struct neighbor *ptrGlobal;
	extern FILE *ptrMyfile;

	fclose(ptrMyfile);
	free(ptrGlobal);

}

struct neighbor* initInterface()
{
    int i;
	struct neighbor *interfacePtr, *firstInterface;

	extern uint32_t ipInterfaceSet[3];
	extern uint64_t macInterfaceSet[3];

//	extern uint32_t ipInterfaceSet[2];
//	extern uint64_t macInterfaceSet[2];
	extern INTERFACECOUNT;



	for (i=0;i<INTERFACECOUNT;i++){

		struct neighbor *interface = malloc(sizeof(struct neighbor));

		interface->IPaddrs = ipInterfaceSet[i];
		interface->MACaddrs = macInterfaceSet[i];
		interface->coHost=NULL;
		interface->next = NULL;


		if (i==0){
			interfacePtr = interface;
			firstInterface = interface;

		}
		else{
			interfacePtr->coHost = interface;
			interfacePtr = interface;



		}

	}

	return firstInterface;
}

void termInterface()
{

}

