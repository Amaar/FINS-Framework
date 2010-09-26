#include "finstypes.h"
#include "ARPfunctions.h"
#include "IP4.h"
#include <inttypes.h>

FILE *ptrMyfile; /**< this variable points to the file which contains all the records */



struct ARPmsg simulateARPrequest(uint32_t ipSenderAddress, uint64_t macSenderAddress)
{
	struct ARPmsg Request;

	//this mimics an incoming request for the host machine

	extern uint32_t ipAddressInterface;


	Request.hardwareType = 1;
	Request.protocolType = 0x0800;
	Request.hardwareAddressLength= 6;
	Request.protocolAddressLength = 4;
	Request.operation = ARPrequestOp;
	Request.senderMACaddrs = macSenderAddress;

	Request.senderIPaddrs = ipSenderAddress;

	Request.operation = 1;

	Request.targetMACaddrs = 0;

	Request.targetIPaddrs = ipAddressInterface;

	return Request;
}

struct finsFrame simulateReply(struct finsFrame rcvdFrame)
{
	extern struct neighbor *ptrGlobal;
	struct neighbor *ptrNode;
	struct ARPmsg requestARP, replyARP, *tempARP;
	struct finsFrame finsFrameReply;

	ptrNode = ptrGlobal;

	tempARP = (struct ARPmsg*) rcvdFrame.dataFrame.pdu;
	requestARP=*tempARP;

	/*	printf("\nTarget MAC address %llu\n", requestARP.targetMACaddrs);
	while (ptrNode!=NULL )
	{
		printf(" MAC address %llu ", ptrNode->MACaddrs);
		ptrNode=ptrNode->next;
		//		if (ptrNode->MACaddrs==requestARP.targetMACaddrs)
		//		printf("HURRAYYYYY!!!");

	}*/

	ptrNode = ptrGlobal;

	requestARP = genARPmsg(rcvdFrame);
	printf("\nWell this is what the simReply receives\n");
	printf("Operation Type = %d", requestARP.operation);
	printARP(requestARP);



	//The following is the default entry
	replyARP.senderIPaddrs = NULLADDRESS;
	replyARP.senderMACaddrs = NULLADDRESS;
	replyARP.targetIPaddrs = NULLADDRESS;
	replyARP.targetMACaddrs = NULLADDRESS;
	replyARP.operation = NULLADDRESS;


	while (ptrNode!=NULL )
	{


		if (ptrNode->IPaddrs==requestARP.targetIPaddrs)
		{
			//			replyARP = generateARPreply(requestARP);

			replyARP.senderIPaddrs = ptrNode->IPaddrs;
			replyARP.senderMACaddrs = ptrNode->MACaddrs;
			replyARP.targetIPaddrs =requestARP.senderIPaddrs;
			replyARP.targetMACaddrs = requestARP.senderMACaddrs;
			replyARP.hardwareAddressLength = requestARP.hardwareAddressLength;
			replyARP.hardwareType = requestARP.hardwareType;
			replyARP.operation = ARPreplyOp;
			replyARP.protocolAddressLength=requestARP.protocolAddressLength;
			replyARP.protocolType=requestARP.protocolType;



		}

		ptrNode = ptrNode->next;
	}



	//	replyARP.operation = 2;
	tempARP=&replyARP;
	printf("\nThis is what ARP reply is generated\n");

	printARP(replyARP);

	finsFrameReply =  genFinsFrame(&replyARP);


	return finsFrameReply;
}

struct neighbor* ReadNeighborList(char* fileName)
		{

	struct neighbor *ptrCT;

	FILE *fp;

	int i,j;

	struct neighbor *pCT, *p2CT, ptemp; /**<These variables are used to store
	the read struct data from the file*/


	if((fp=fopen(fileName, "r")) == NULL) {
		printf("Cannot open file.\n");
		exit(1);
	}

	j = 0;
	i = 0;

	while (!feof(fp))
	{
		//read line from file
		pCT = (struct neighbor *) malloc (sizeof(struct neighbor));

		fread(&ptemp,sizeof(struct neighbor),1,fp);

		pCT->IPaddrs = ptemp.IPaddrs;
		pCT->MACaddrs=ptemp.MACaddrs;
		pCT->next = NULL;

		if (j==0)
		{ p2CT = pCT;
		ptrCT = pCT;
		}
		else
		{
			p2CT->next = pCT;
			p2CT = pCT;
		}

		j=j+1;
	}

	return ptrCT;
		}

/** @brief Populates a binary file with artificially generated table records, The function can be used to create randomly generated FINS control frame (inbound or outbound) for any module

@param FileName is the name of the file
 */

void ProduceNeighborList(char *FileName)
{


	int neighborNum, J; /**< This is a variables for iterating through number of records <recordsNum>*/

	uint8_t IPa, IPb, IPc, IPd, MACa, MACb, MACc, MACd, MACe, MACf;

	struct neighbor Rec;


	FILE *fp;

	fp=fopen(FileName,"w");
	if (!fp)                               //open file for writing , exit if it cannot open
	{
		printf("Unable to open file!");
		exit (0);
	}


	printf("How many neighbors ?\n");

	scanf("%d", &neighborNum);

	srand ( (unsigned)time (0) );


	for (J=0;J<neighborNum;J++)
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


		Rec.IPaddrs = genIPaddress(IPa,IPb,IPc,IPd);

		Rec.MACaddrs = genMACaddress(MACa,MACb,MACc,MACd,MACe, MACf);


		fwrite(&Rec, sizeof(struct neighbor), 1, fp);     //the FINS control frame is then written to the opened binary file as a structure


	}//generate for all the records (ends for loop)



	fclose(fp);    // closes the file

}



void senderTest(char *fileName) //test network response
{

	extern struct neighbor *ptrGlobal;
	struct finsFrame *dataARP, dataARP2, neighborReply;
	struct ARPmsg *msg, msg2, msg3, msg4, *temp;

	struct neighbor *node, *ptrCache, *tempNeighbor;


	extern uint64_t macAddressInterface;
	extern uint32_t ipAddressInterface;



	uint32_t IPt;

	// node = NULL;
	ptrCache = NULL;
	tempNeighbor = NULL;

	printf("\nSending Node Test\n\n");


	//    msg = generateARPrequest(9,array);

	ProduceNeighborList(fileName);

	ARPInitialize(fileName);

	printNeighbors(ptrGlobal);


	printf("\n**************\ntesting functions\n\n");

	while (IPt!=0)
	{
		IPt = readIPaddress();

		//    printIPaddress(IPt);
		msg2 = generateARPrequest(IPt);
		// = generateARPreply(msg2);

		//  printARP(msg2);

		msg = &msg2;
		dataARP2 = genFinsFrame(msg);

		msg4 = genARPmsg(dataARP2);
		//   dataARP = &dataARP2;
		//temp = (struct ARPmsg*) dataARP2.dataFrame.pdu;

		//	printf("\n\nTesting genARPmsg in the main loop\n\n\n");

		//msg2 = *temp;
		printARP(msg4);

		//	  searchNeighbor()

		// dataARP2 = genFinsFrame(&msg2);

		neighborReply = simulateReply(dataARP2);


		msg4 = genARPmsg(neighborReply);


		tempNeighbor = updateCache(ptrCache, msg4);

		ptrCache = tempNeighbor;

		printNeighbors(ptrCache);


	}



	ARPTerminate();

}

void receiverTest(char *fileName)// test node response to received packet
{

	extern struct neighbor *ptrGlobal;
	struct finsFrame *dataARP, dataARP2, neighborReply;
	struct ARPmsg *msg, msg2, msg3, msg4, *temp;

	struct neighbor *node, *ptrCache, *tempNeighbor;

	extern uint64_t macAddressInterface, MACADDRESS, MACADDRESS2, MACADDRESS3;
	extern uint32_t ipAddressInterface, IPADDRESS, IPADDRESS2, IPADDRESS3;
    int interfaceNum;


	uint32_t IPt;
	uint64_t macSenderAddress;

	// node = NULL;
	ptrCache = NULL;
	tempNeighbor = NULL;

	printf("\nReceiving Node Test\n\n");


	//    msg = generateARPrequest(9,array);

	ProduceNeighborList(fileName);

	ARPInitialize(fileName);

	printNeighbors(ptrGlobal);

	ptrCache = initInterface();


	printf("Now Printing the Interface information \n");
	printLocalCache(ptrCache);

	printf("\n*****From the above list choose which node should generate the ARP message?\n\n");
	fflush(stdout);


	while (IPt!=0)
	{
		IPt = readIPaddress();

		//    printIPaddress(IPt);

		macSenderAddress = searchMACaddress(IPt, ptrGlobal);

		printf("\n*****Which interface (1,2,3) should be receiving it (testing it of course)?\n\n");
		fflush(stdout);
		scanf("%d", &interfaceNum);


		if (interfaceNum == 1){
		macAddressInterface = MACADDRESS;
    	ipAddressInterface = IPADDRESS;}
		else if (interfaceNum == 2){
			macAddressInterface = MACADDRESS2;
			ipAddressInterface = IPADDRESS2;

		}
		else if (interfaceNum == 3){
			macAddressInterface = MACADDRESS3;
			ipAddressInterface = IPADDRESS3;

		}


		if (macSenderAddress!=NULLADDRESS)
		{msg2= simulateARPrequest(IPt, macSenderAddress);


		printARP(msg2);

		//based on the received frame simply add that node to your list


		tempNeighbor = updateCache(ptrCache, msg2);


		ptrCache = tempNeighbor;

		printLocalCache(ptrCache);
		}


	}


	ARPTerminate();


}


