/**@file arp.h
 *@brief this files contains all relevant data types and prototypes of the functions for an ARP module
 *@author Syed Amaar Ahmad
 *@date  September 27, 2010
 */

#include "finstypes.h"
#include <inttypes.h>

#define ARPREQUESTOP 1
#define ARPREPLYOP 2
#define NULLADDRESS 0
#define HWDTYPE 1
#define PROTOCOLTYPE 0x800
#define HDWADDRSLEN 6
#define PROTOCOLADDRSLEN 4
#define ARPMSGLENGTH 208

struct ARP_message
{
	uint16_t hardware_type;
	uint16_t protocol_type;
	uint8_t hardware_addrs_length;
	uint8_t protocol_addrs_length;
	uint16_t operation;
	uint64_t sender_MAC_addrs;
	uint32_t sender_IP_addrs;
	uint64_t target_MAC_addrs;
	uint32_t target_IP_addrs;

};

struct node{

	uint64_t MAC_addrs;
	uint32_t IP_addrs;

	struct node *next;
	struct node *co_intface;

};

void print_IP_addrs(uint32_t ip_addrs);

void print_MAC_addrs(uint64_t mac_addrs);

uint32_t gen_IP_addrs(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

uint64_t gen_MAC_addrs(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f);

struct ARP_message gen_requestARP(uint32_t ip_target_addrs);

struct ARP_message gen_replyARP(struct ARP_message request);

int search_list(struct node *ptr_to_cache, uint64_t MAC_addrs);

void update_cache(struct ARP_message reply);

void print_msgARP(struct ARP_message);

void print_neighbors(struct node *ptr_to_cache);

void print_cache();

uint64_t search_MAC_addrs(uint32_t IP_addrs, struct node *ptr);

struct finsFrame arp_to_fins(struct ARP_message *rcvd_frame);

struct ARP_message fins_to_arp(struct finsFrame);

struct node* init_intface();

void term_intface();






