#ifndef ASSIGN3_H
#define ASSIGN3_H

#define NUM_VARS 25
#define MALLOC 1
#define FREE 2
#define REALLOC 3
#define BEST_FIT 1
#define WORST_FIT 2
#define FIRST_FIT 3
#define NEXT_FIT 4

struct memNode {
    int isFree;
    int start;
    int size;
    struct memNode *prev;
    struct memNode *next;
};

/* Function prototypes */
struct memNode *makeNode(int isfree, int start, int size, struct memNode *prev, struct memNode *next);
void printList(struct memNode *p);
void printVars(struct memNode *p[]);
void split(struct memNode *p, int size);
void coalesce(struct memNode *p);
struct memNode *findFree(struct memNode *h, int size, int algo);
void freeList(struct memNode *h);

#endif