#include "finstypes.h"
#include <inttypes.h>

struct finsFrame simulateReply(struct finsFrame rcvdFrame);

struct ARPmsg simulateARPrequest(uint32_t ipSenderAddress, uint64_t macSenderAddress);

struct neighbor* ReadNeighborList(char* fileName);

void ProduceNeighborList(char* fileName);

void senderTest(char *fileName); //test network response

void receiverTest(char *fileName);// test node response to received packet


