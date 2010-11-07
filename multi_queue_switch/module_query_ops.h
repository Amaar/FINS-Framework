/**
 * module_query_ops.h
 *
 *  Created on: Nov 3, 2010
 *      Author: amaar
 *      Note that this file is also needed by the switch to carry out some functions.
 */

#include "finstypes.h"

#define GENQUERY 1
#define UPDATECACHE 2
#define SEARCHLOCALTABLE 3

unsigned int serial_num; /**<used by a module to keep track of the id of the query fins frame*/

struct tableRecord *module_cache_ptr; /**<A module's table record pointer to cache (in heap)*/

void gen_record(struct tableRecord *rec, unsigned char sourceId, unsigned char destinationID, unsigned char vci, unsigned char directionFlag);
void produce_table(char *FileName);
struct tableRecord* read_table(char* fileName);

void init_module_cache();
void gen_query(unsigned char ModuleId, struct finsFrame *query_request);
void free_records(struct tableRecord *ptr);
void print_records(struct tableRecord *ptr);
int is_record_unique(struct tableRecord *Pu1, struct tableRecord *Pu2);
struct tableRecord* connect_two_lists(struct tableRecord *P, struct tableRecord *P1);
struct tableRecord* update_module_cache(struct finsFrame *QueryReply, struct tableRecord *PTRX);
struct destinationList * search_module_table(struct tableRecord *ptrCache, unsigned char vc, unsigned char DF);
void print_dest_list (struct destinationList *dList);
void free_dest_list(struct destinationList *ptr);
int len_dst_list(struct destinationList *ptr);








