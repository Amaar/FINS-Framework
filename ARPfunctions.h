#include "finstypes.h"
//#include "ARP.h"

#include <inttypes.h>

#define ARPrequestOp 1
#define ARPreplyOp 2
#define NULLADDRESS 0

struct ARPmsg
{
	uint16_t hardwareType;
	uint16_t protocolType;
	uint8_t hardwareAddressLength;
	uint8_t protocolAddressLength;
	uint16_t operation;
	uint64_t senderMACaddrs;
	uint32_t senderIPaddrs;
	uint64_t targetMACaddrs;
	uint32_t targetIPaddrs;

};

struct neighbor{

	uint64_t MACaddrs;
	uint32_t IPaddrs;

	struct neighbor *next;

	struct neighbor *coHost;

};



//int32_t genIPaddress(int a, int b, int c, int d);
void printIPaddress(uint32_t ip);

//int64_t genMACaddress(int a, int b, int c, int d, int e);
void printMACaddress(uint64_t mac);

uint32_t genIPaddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

uint64_t genMACaddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f);


struct ARPmsg generateARPrequest(uint32_t ipTargetAddress);

uint32_t readIPaddress();

//fill up with my MAC address/IP address and send back
struct ARPmsg generateARPreply(struct ARPmsg Request);

int searchNeighbor(struct neighbor *ptrCache, uint64_t MACaddress);

struct neighbor * updateCache(struct neighbor *ptrCache, struct ARPmsg N);


void printARP(struct ARPmsg);

void printNeighbors(struct neighbor *Node);

void printLocalCache(struct neighbor *Node);

uint64_t searchMACaddress(uint32_t IPaddress, struct neighbor *node);


void genMessages();

void termARP();

struct finsFrame genFinsFrame(struct ARPmsg *rcvdFrame);

struct ARPmsg genARPmsg(struct finsFrame);

void ARPInitialize(char *fileName);

void ARPTerminate();


struct neighbor* initInterface();
void termInterface();






