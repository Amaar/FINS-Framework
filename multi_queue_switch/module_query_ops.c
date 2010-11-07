/*
 * module_query_ops.c
 *
 *  Created on: Nov 3, 2010
 *      Author: amaar
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "finstypes.h"
#include "module_query_ops.h"
#include "semaphore.h"
#include "finsdebug.h"
#include "metadata.h"

#define DEBUG

/**@brief creates a fins control for either request of reply query
 * @param task refers to whether it is generate request, or update cache
 * @param module_id is identity of the module
 * @param pckt is a pointer to a fins control frame which can be both request/reply query.
 * @param state is used to set/reset the local cache to NULL. It MUST be ZERO 0 for the very first query request
 */
void module_query_task(int task, unsigned char module_id, struct finsFrame *pckt, int state){

	struct tableRecord *cache_ptr_temp;//temporary pointer to the cache of a module

	if (state == 0){
		module_cache_ptr = NULL;
		serial_num = 0;
		}
	else
		serial_num = serial_num + 1;

	if (task==GENQUERY)

		gen_query(module_id, pckt);

	else if (task==UPDATECACHE){

		cache_ptr_temp = module_cache_ptr;
		module_cache_ptr = update_module_cache(pckt, cache_ptr_temp);
	}
}

/**@brief inserts the correct values into a fins query frame
 *  @param ModuleId is the querying module's ID (e.g. 22 for IP )
 *  @param query_request is the pointer to the fins frame which will be filled with these values
 */
void gen_query(unsigned char ModuleId, struct finsFrame *query_request)
{
	query_request->ctrlFrame.senderID = ModuleId;
	query_request->ctrlFrame.opcode = QUERYREQUEST;
	query_request->destinationID.id = SWITCHID; // SWITCH IS THE DESTINATION
	query_request->destinationID.next = NULL;
	query_request->dataOrCtrl=CONTROL;
	query_request->ctrlFrame.serialNum = serial_num;
	serial_num = serial_num + 1; //incremented every time a query request is made by the module

	query_request->ctrlFrame.paramterID = 0;
	query_request->ctrlFrame.paramterValue = NULL;
	query_request->ctrlFrame.replyRecord = NULL;

};

/**
@brief sends back tableRecord based on its input parameters
@param senderId is the identity of the sending module
@param destinationID is the identity of the receiving module
@param vci
@param directionFlag is whether it is in or out
 */
void gen_record(struct tableRecord *rec, unsigned char sourceId, unsigned char destinationID, unsigned char vci, unsigned char directionFlag)
{
	rec->destinationID = destinationID;
	rec->sourceID = sourceId;
	rec->vci = vci;
	rec->directionFlag = directionFlag;
	rec->next = NULL;
}

/**
@brief removes the dynamic linked list to prevent memory leakage
@param ptr is a pointer to the first record of this list
 */

void free_records(struct tableRecord *ptr_tab)
{
	struct tableRecord *ptr, *ptr2;

	ptr = ptr_tab;
	ptr2 = ptr;

	while (ptr!=NULL){
		ptr2 = ptr->next;
		free(ptr);
		ptr = ptr2;
	}
}


/**
@brief checks whether the record pointed to by the ptr exists within a linked list which starts by PTRX
@param tab_ptr1 is a pointer to the list of records
@param tab_ptr2 is a pointer to the record being tested for uniqueness
 */

int is_record_unique(struct tableRecord *tab_ptr1, struct tableRecord *tab_ptr2){

	/** This function determines how many matching records already exist within the linked list
	 *
	 * */
	struct tableRecord *tab_ptr3;
	int i = 0;

	tab_ptr3 = tab_ptr1;

	while (tab_ptr3!=NULL)
	{
		if (tab_ptr2->destinationID==tab_ptr3->destinationID && tab_ptr2->sourceID==tab_ptr3->sourceID && tab_ptr2->vci==tab_ptr3->vci && tab_ptr2->directionFlag==tab_ptr3->directionFlag)
			i=i+1;
		tab_ptr3= tab_ptr3->next;
	}

	return i;
}

/**
@brief connects two linked lists together; the second list is appended after the first list
@param tab_ptr is a pointer to the first record of the first list
@param tab_ptr1 is a pointer to the first record of the second list
 */
struct tableRecord* connect_two_lists(struct tableRecord *tab_ptr, struct tableRecord *tab_ptr1){

/**this function simply joins/connects two linked lists tab_ptr and tab_ptr1 and
 *
  returns the pointer to the new list's header*/

	struct tableRecord *tab_ptr2, *tab_ptr3;

	//tab_ptr1 is the old list
	// P is the new list and needs to be added


	if (tab_ptr1==NULL)  //if old list is null and does not exist

		if (tab_ptr==NULL)
		tab_ptr2= NULL;

		else
		tab_ptr2 = tab_ptr;   //new list becomes the current list

	else

	{
		if (tab_ptr==NULL)  //if new list is null and does not exist
			tab_ptr2 = tab_ptr1;  //return old list only
		else
		{
			tab_ptr2 = tab_ptr1;
			tab_ptr3 = tab_ptr1;

			while (tab_ptr3->next !=NULL)   //append the two lists sequentially and return
				tab_ptr3= tab_ptr3->next;

			tab_ptr3->next = tab_ptr;
			tab_ptr1 = tab_ptr2;
		}
	}
	return tab_ptr2;
}

/**
@brief updates a linked list at the side of the querying module, also makes use of ConnectLists function (see below)
@param query_reply is the query reply and has the linked list (which is going to be appended with the module's local cache)
@param cache_ptr is the pointer to the first element of the local cache at the module side
 */
struct tableRecord* update_module_cache(struct finsFrame *query_reply, struct tableRecord *cache_ptr){

	/**returns pointer to the new linked list created from the searched linked list*/

	struct tableRecord *ptr, *ptr_new_list, *pUpdatedCache, *pUniqueTest;
	int found, unique;
	struct tableRecord *first_rec = (struct tableRecord *) malloc (sizeof(struct tableRecord));

	ptr = query_reply->ctrlFrame.replyRecord;

	found = 0;

	if (query_reply->ctrlFrame.replyRecord!=NULL)   //If at least one record was found from the Table
	{

		ptr_new_list = first_rec;

		while (ptr!=NULL)
		{
			pUniqueTest = cache_ptr;

			unique = is_record_unique(pUniqueTest, ptr); /**check if any such record (ptr) exists within the current cache (pUniqueTest) itself*/

			if (ptr!=NULL && unique == 0) /**Update cache with the new list only if the entries are unique*/
			{

				if (found==0)
				{
					first_rec->sourceID = ptr->sourceID;
					first_rec->destinationID = ptr->destinationID;
					first_rec->vci = ptr->vci;
					first_rec->directionFlag = ptr->directionFlag;
					first_rec->next = NULL;
					ptr_new_list = first_rec;
					pUpdatedCache = connect_two_lists(first_rec, cache_ptr);
					cache_ptr = pUpdatedCache;
				}
				else if (found>0)
				{
					struct tableRecord *later_rec = (struct tableRecord *) malloc (sizeof(struct tableRecord));
					later_rec->sourceID = ptr->sourceID;
					later_rec->destinationID = ptr->destinationID;
					later_rec->vci = ptr->vci;
					later_rec->directionFlag = ptr->directionFlag;
					later_rec->next = NULL;
//					ptr_new_list->next = later_rec;
//					ptr_new_list = later_rec;
					pUpdatedCache = connect_two_lists(later_rec, cache_ptr);
					cache_ptr = pUpdatedCache;
				}

				found=found+1;
			} //end if (ptr!=NULL && Unique == 0) condition

			ptr=ptr->next; //next record in the query reply linked list
		}//end while loop

	//	if (found!=0)
	//		pUpdatedCache = connect_two_lists(first_rec, cache_ptr);
	//	else
		if (found==0)
		{
			pUpdatedCache = connect_two_lists(NULL, cache_ptr);//if only matched entries were found then the cache stays the same (the elements already exist at the cache)
			free(first_rec);//if the cache does not require an update the heap is freed
		}
	}
	else
	{
		free(first_rec);//if the cache does not require an update the heap is freed
		pUpdatedCache = connect_two_lists(NULL, cache_ptr);
	}

	free_records(query_reply->ctrlFrame.replyRecord);  /*free memory which had been allocated for the initial searching*/

	return pUpdatedCache;
}


/**
@brief checks the local cache of a module and return the destination list found within the cache.
@param ptrCache is a pointer to the first record of the cache
@param vc is the VCI
@param DF is the directionFlag
 */

struct destinationList * search_module_table(struct tableRecord *ptrCache, unsigned char vc, unsigned char DF){

	struct tableRecord *ptr1;

	struct destinationList *tempDst;

	struct destinationList *First = (struct destinationList *) malloc (sizeof(struct destinationList));

	int L = 0;

	ptr1 = ptrCache;

	while (ptr1!=NULL) //while the local cache is still being searched
	{

		if ((ptr1->vci == vc) && (ptr1->directionFlag == DF) ) //match the entries
		{

			if (L == 0)   //for the very first destination ID
			{

				First->id = ptr1->destinationID;
				First->next = NULL;
				tempDst = First;

			}
			else  //for all other destination IDs
			{
				struct destinationList *Later = (struct destinationList *) malloc (sizeof(struct destinationList));
				Later->id = ptr1->destinationID;
				Later->next = NULL;
				tempDst->next = Later;
				tempDst = Later;
			}

			L= L+1;

		} //end if of the `match the entries'


		ptr1= ptr1->next;

	} //ending the cache searching


	if (L==0) {
		free(First); /**if no match was found or empty cache then the memory has to be freed*/
		return NULL;
	}
	else
		return First;

}

/**
@brief prints the destination list as searched in the local cache
@param dList points to the first element of the found list
 */
void print_dest_list (struct destinationList *dList)
{
	while (dList!=NULL){
		PRINT_DEBUG("VCI-based, Direction Flag-based Destination ID : %d \n",dList->id);
		dList = dList ->next;
	}
}

/**
@brief liberates  the memory used by the destination list
@param ptr points to the destination list
 */

void free_dest_list(struct destinationList *ptr)
{
	struct destinationList *ptrd2;
	ptrd2=ptr;
	while (ptr!=NULL){
		ptrd2=ptr->next;
		free(ptr);
		ptr=ptrd2;
	}
}

/**
@brief returns the number of elements in the linked list
@param ptr points to the destination list
 */
int len_dst_list(struct destinationList *ptr)
{
	int ListLength = 0;
	struct destinationList *ptr2;

	ptr2 = ptr;

	while(ptr2!=NULL){
		ListLength = ListLength + 1;
		ptr2 = ptr2->next;
	}
	return ListLength;
}

/** @brief reads from a file which has stored a table of records

@param fileName is the name of the file
 */

struct tableRecord* read_table(char* fileName){

	FILE *fp;/**<file pointer containing config file*/
	int j; /**temporary variables*/

	struct tableRecord *first_rec, *later_rec, p_temp; /**<These variables are used to store
	the read struct data from the file*/

	if((fp=fopen(fileName, "r")) == NULL) {
		PRINT_DEBUG("Cannot open file.\n");
		exit(1);
	}

	j = 0;

	while (!feof(fp))
	{
		struct tableRecord *rec = (struct tableRecord *) malloc (sizeof(struct tableRecord));
		fread(&p_temp,sizeof(struct tableRecord),1,fp);

		rec->sourceID = p_temp.sourceID;
		rec->destinationID = p_temp.destinationID;
		rec->vci = p_temp.vci;
		rec->directionFlag = p_temp.directionFlag;
		rec->next = NULL;

		if (j==0)
		{ later_rec = rec;
		  first_rec = rec;
		}
		else
		{
			later_rec->next = rec;
			later_rec = rec;
		}
		j=j+1;
	}

	return first_rec;
}



/** @brief Populates a binary file with artificially generated table records, The function can be used to create randomly generated FINS control frame (inbound or outbound) for any module

@param FileName is the name of the file
 */

void produce_table(char *FileName)
{

	int records_num, num_modules, j; /**< This is a variables for iterating through number of records <recordsNum>*/
	unsigned char sId, dId, vci, in_out;

	struct tableRecord rec;

	FILE *fp;

	num_modules = 6; //number of modules in the system

	fp=fopen(FileName,"w");
	if (!fp)                               //open file for writing , exit if it cannot open
	{
		PRINT_DEBUG("Unable to open file!");
		exit (0);
	}


	PRINT_DEBUG("How many records to create ?\n");
	scanf("%d", &records_num);

	srand ( (unsigned)time (0) );
	for (j=0;j<records_num;j++)
	{

		sId = (rand()%num_modules+1)*11;
		dId = (rand()%num_modules+1)*11; //random module IDs being generated

		if (sId==sId)   //this loop ensures that the destination and source ID are not the same
		{
			sId = (rand()%num_modules+1)*11;

			while (sId==dId)
				sId = (rand()%num_modules+1)*11;
		}

		vci = (rand());
		in_out = (rand()%2);

		gen_record(&rec,(unsigned char) sId, (unsigned char) dId, (unsigned char) vci, (unsigned char) in_out);

		fwrite(&rec, sizeof(struct tableRecord), 1, fp);     //the FINS control frame is then written to the opened binary file as a structure
	}//generate for all the records (ends for loop)

	fclose(fp);    // closes the file
}


/**
@brief prints the contents of the linked list
@param ptr is a pointer to the first record of this list
 */

void print_records(struct tableRecord *ptr)
{
	struct tableRecord *ptr_dup;

	ptr_dup = ptr;
	PRINT_DEBUG("\n ********* Start Printing tableRecords **********\n");
	while (ptr_dup!=NULL){
		PRINT_DEBUG("\n");
		PRINT_DEBUG("Sender Id %d \n ",ptr_dup->sourceID);
		PRINT_DEBUG("vci %d \n",ptr_dup->vci);
		PRINT_DEBUG("Direction Flag %d \n",ptr_dup->directionFlag);
		PRINT_DEBUG("Destination Id %d \n",ptr_dup->destinationID);
		ptr_dup = ptr_dup->next;
	}
	PRINT_DEBUG("\n ********* Finished Printing tableRecords **********\n");
}
