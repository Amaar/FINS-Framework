#include "IP4.h"
#include "finstypes.h"
//#include "ARP.h"
#include "ARPfunctions.h"
#include "testHarnessARP.h"

int INTERFACECOUNT= 3;

struct neighbor *ptrGlobal;


uint64_t MACADDRESS = 325673433;
uint32_t IPADDRESS = 672121;

uint64_t MACADDRESS2 = 234233;
uint32_t IPADDRESS2 = 324867040;

uint64_t MACADDRESS3 = 81100911;
uint32_t IPADDRESS3 = 890006;


uint32_t ipInterfaceSet[3];
uint64_t macInterfaceSet[3];

uint64_t macAddressInterface;
uint32_t ipAddressInterface;


int main(int argc, char *argv[])
{

	//	senderTest(argv[1]); //test network response

	ipInterfaceSet[0] = IPADDRESS;
	ipInterfaceSet[1] = IPADDRESS2;
	ipInterfaceSet[2] = IPADDRESS3;

	macInterfaceSet[0] = MACADDRESS;
	macInterfaceSet[1] = MACADDRESS2;
	macInterfaceSet[2] = MACADDRESS3;

	macAddressInterface = MACADDRESS2;
	ipAddressInterface = IPADDRESS2;


//OR
	receiverTest(argv[1]);



	/*	struct finsFrame *dataARP, dataARP2, neighborReply;
    struct ARPmsg *msg, msg2, msg3, msg4, *temp;

    struct neighbor *node, *ptrCache, *tempNeighbor;

    uint32_t IPt;

    node = NULL;
    ptrCache = NULL;
    tempNeighbor = NULL;


//	packetARP.pdu = (char *) dataARP;

            int array[9];
    		array[0]=1;
    		array[1]=2;
    		array[2]=5;
    		array[3]=6;
    		array[4]=11;
    		array[5]=88;
    		array[6]=4432;
    		array[7]=673;
    		array[8]=85;

//    msg = generateARPrequest(9,array);



    ProduceNeighborList(argv[1]);

    ARPInitialize(argv[1]);

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

	printf("\n\nTesting genARPmsg in the main loop\n\n\n");

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



	 ARPTerminate();*/

	return 0;
}
