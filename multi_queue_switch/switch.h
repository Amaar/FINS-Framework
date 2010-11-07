/*
 * switch.h
 *
 *  Created on: Nov 4, 2010
 *      Author: amaar
 */

#ifndef SWITCH_H_
#define SWITCH_H_


#endif /* SWITCH_H_ */

#include "finstypes.h"
#include "module_query_ops.h"
#include <semaphore.h>
#include <sys/sem.h>
#include <pthread.h>    /* POSIX Threads */

/**Note that if more modules are added then for each module two corresponding queues need to be declared
 * in the following code. These unidirectional queues store and forward fins frames to and from the switch.
 * It is the job of the init_switch() function to initialize the switch and the queues. Note that the switch
 * basically controls the whole process.*/

struct queue *ip_to_swt_bff;/**<queue from IP module to switch*/
struct queue *swt_to_ip_bff;/**<queue from  switch to IP module*/
struct queue *tcp_to_swt_bff; /**<queue from tcp to switch */
struct queue *swt_to_tcp_bff;/**<queue from switch to tcp*/
struct queue *wifi_to_swt_bff; /**<queue from ethernet stub to switch*/
struct queue *swt_to_wifi_bff; /**<queue from switch to ethernet stub*/
struct queue *arp_to_swt_bff; /**<queue from arp to switch */
struct queue *swt_to_arp_bff;/**<queue from switch to arp*/
struct queue *module_to_switch_bff; /**<general pointer to any queue representing a module-to-switch flow*/
struct queue *switch_to_module_bff; /**<general pointer to any queue representing a switch-to-module flow*/

struct tableRecord *switch_table_ptr; /**<points to the switch's copy of table records*/

void init_switch(char *file_name);//file_name is the name of the configuration file

void term_switch();

void gen_query_reply(struct finsFrame *reply, struct finsFrame *request, struct tableRecord *search_table_ptr);

void fins_switch_in();

void fins_switch_out(struct finsFrame *fins_out);


