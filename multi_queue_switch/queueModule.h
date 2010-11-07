
#include "finstypes.h"
#include <semaphore.h>
#include <sys/sem.h>
#include <pthread.h>    /* POSIX Threads */

struct q_element{

struct finsFrame element;
struct q_element *next;

};

struct queue{

struct q_element *head, *tail;
int num_elements;
sem_t locked;
};

struct queue* init_queue();

void term_queue(struct queue *);

int write_queue(struct finsFrame *, struct queue *);

int read_queue(struct finsFrame *, struct queue *);

void cpy_fins_to_fins(struct finsFrame *src, struct finsFrame *dst);

void print_finsFrame(struct finsFrame *fins_in);


