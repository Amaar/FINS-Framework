/**@file test_arp.h
 *@brief this files contains all prototypes for the functions to test an ARP module
 *@author Syed Amaar Ahmad
 *@date  September 27, 2010
 */

#include "finstypes.h"
#include <inttypes.h>

void gen_neighbor_list(char* fileName);

struct ARP_message mimic_net_request(uint32_t IP_sender_addrs, uint64_t MAC_sender_addrs);

struct finsFrame mimic_net_reply(struct finsFrame reply_fins);

struct node* read_neighbor_list(char* fileName);

void init_recordsARP(char *fileName);

void sender_test(char *fileName);

void receiver_test(char *fileName);

uint32_t read_IP_addrs();


