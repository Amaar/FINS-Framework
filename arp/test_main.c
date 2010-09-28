/**@file test_main.h
 *@brief this files is the main file to test functions of the ARP module with SINGLE INTERFACE
 *@brief More specifically it provides the IP and MAC addresses of the host's interface
 *@author Syed Amaar Ahmad
 *@date  September 27, 2010
 */
#include "finstypes.h"
#include "arp.h"
#include "test_arp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int INTERFACECOUNT= 1;

uint64_t MACADDRESS = 325673433;/**<hard coded MAC address of host*/
uint32_t IPADDRESS = 672121;/**<hard coded IP address of host*/
uint32_t IP_interface_set[1];
uint64_t MAC_interface_set[1];
uint64_t interface_MAC_addrs;/**<MAC address of current interface*/
uint32_t interface_IP_addrs;/**<IP address of current interface*/

int main(int argc, char *argv[])
{
	IP_interface_set[0] = IPADDRESS;
	MAC_interface_set[0] = MACADDRESS;
	interface_MAC_addrs = MACADDRESS;
	interface_IP_addrs = IPADDRESS;

	receiver_test(argv[1]); //this tests sending ARP messages
	sender_test(argv[1]); //this tests receiving ARP messages

	return 0;
}
