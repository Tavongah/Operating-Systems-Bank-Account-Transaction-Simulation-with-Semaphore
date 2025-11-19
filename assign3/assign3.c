#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static struct memNode *head = NULL;
static struct memNode *nextFitStart = NULL;

/* Create a new memNode */
struct memNode *makeNode(int isfree, int start, int size, struct memNode *prev, struct memNode *next) {
    struct memNode *n = (struct memNode *)malloc(sizeof(struct memNode));
    if (!n) {
        fprintf(stderr, "Memory allocation failure\n");
        exit(1);
    }
    n->isFree = isfree;
    n->start = start;
    n->size = size;
    n->prev = prev;
    n->next = next;
    return n;
}

/* Print memory list in the expected format */
void printList(struct memNode *p) {
    printf("List:\n");
    struct memNode *cur = p;
    while (cur) {
        printf("(%d,%d,%d)\n", cur->isFree, cur->start, cur->size);
        cur = cur->next;
    }
}

/* Print variable list in the expected format */
void printVars(struct memNode *p[]) {
    printf("Vars:\n");
    for (int i = 0; i < NUM_VARS; ++i) {
        if (p[i]) {
            printf("(%d,%d,%d)\n", i, p[i]->start, p[i]->size);
        }
    }
}

/* Helper: split node p into an allocated chunk of size 'size' at beginning */
void split(struct memNode *p, int size) {
    if (!p || p->size < size) return;

    if (p->size == size) {
        p->isFree = 0;
        return;
    }

    int remainderStart = p->start + size;
    int remainderSize = p->size - size;

    struct memNode *remainder = makeNode(1, remainderStart, remainderSize, p, p->next);
    if (p->next) p->next->prev = remainder;
    p->next = remainder;
    p->size = size;
    p->isFree = 0;
}

/* Coalesce node p with adjacent free nodes */
void coalesce(struct memNode *p) {
    if (!p) return;

    /* Merge with previous if free */
    if (p->prev && p->prev->isFree) {
        struct memNode *prev = p->prev;
        prev->size += p->size;
        prev->next = p->next;
        if (p->next) p->next->prev = prev;
        free(p);
        p = prev;
    }

    /* Merge with next if free */
    if (p->next && p->next->isFree) {
        struct memNode *nextn = p->next;
        p->size += nextn->size;
        p->next = nextn->next;
        if (nextn->next) nextn->next->prev = p;
        free(nextn);
    }
}

/* Find a free node according to the chosen algorithm */
struct memNode *findFree(struct memNode *h, int size, int algo) {
    if (!h) return NULL;

    struct memNode *cur;
    struct memNode *best = NULL;

    if (algo == BEST_FIT) {
        cur = h;
        while (cur) {
            if (cur->isFree && cur->size >= size) {
                if (!best || cur->size < best->size) best = cur;
            }
            cur = cur->next;
        }
        return best;
    } else if (algo == WORST_FIT) {
        cur = h;
        while (cur) {
            if (cur->isFree && cur->size >= size) {
                if (!best || cur->size > best->size) best = cur;
            }
            cur = cur->next;
        }
        return best;
    } else if (algo == FIRST_FIT) {
        cur = h;
        while (cur) {
            if (cur->isFree && cur->size >= size) return cur;
            cur = cur->next;
        }
        return NULL;
    } else if (algo == NEXT_FIT) {
        if (!nextFitStart) nextFitStart = h;

        cur = nextFitStart;
        struct memNode *firstCandidate = NULL;
        
        while (cur) {
            if (cur->isFree && cur->size >= size) {
                firstCandidate = cur;
                break;
            }
            cur = cur->next;
        }
        
        if (!firstCandidate) {
            cur = h;
            while (cur && cur != nextFitStart) {
                if (cur->isFree && cur->size >= size) {
                    firstCandidate = cur;
                    break;
                }
                cur = cur->next;
            }
        }
        
        if (firstCandidate) {
            nextFitStart = firstCandidate->next ? firstCandidate->next : h;
            return firstCandidate;
        }
        return NULL;
    }

    return NULL;
}

/* Free the entire list */
void freeList(struct memNode *h) {
    struct memNode *cur = h;
    while (cur) {
        struct memNode *n = cur->next;
        free(cur);
        cur = n;
    }
}

/* Main simulation */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    struct memNode *vars[NUM_VARS];
    for (int i = 0; i < NUM_VARS; ++i) vars[i] = NULL;

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Unable to open file %s\n", argv[1]);
        return 1;
    }

    int memSize;
    if (fscanf(fp, "%d", &memSize) != 1) {
        fprintf(stderr, "Bad input: missing memory size\n");
        fclose(fp);
        return 1;
    }

    int algo;
    if (fscanf(fp, "%d", &algo) != 1) {
        fprintf(stderr, "Bad input: missing algorithm\n");
        fclose(fp);
        return 1;
    }

    head = makeNode(1, 0, memSize, NULL, NULL);
    nextFitStart = head;

    int op;
    int errorOutOfSpace = 0;

    while (fscanf(fp, "%d", &op) == 1) {
        if (op == -1) break;

        if (op == MALLOC) {
            int var, size;
            if (fscanf(fp, "%d %d", &var, &size) != 2) {
                fprintf(stderr, "Bad malloc request format\n");
                break;
            }
            if (var < 0 || var >= NUM_VARS) {
                fprintf(stderr, "Variable out of range: %d\n", var);
                continue;
            }

            struct memNode *freeNode = findFree(head, size, algo);
            if (!freeNode) {
                printf("ERROR: Out of Space\n");
                errorOutOfSpace = 1;
                break;
            }

            split(freeNode, size);
            vars[var] = freeNode;

        } else if (op == FREE) {
            int var;
            if (fscanf(fp, "%d", &var) != 1) {
                fprintf(stderr, "Bad free request format\n");
                break;
            }
            if (var < 0 || var >= NUM_VARS) {
                fprintf(stderr, "Variable out of range: %d\n", var);
                continue;
            }
            if (!vars[var]) {
                continue;
            }
            vars[var]->isFree = 1;
            coalesce(vars[var]);
            vars[var] = NULL;
        } else if (op == REALLOC) {
            int var, size;
            if (fscanf(fp, "%d %d", &var, &size) != 2) {
                fprintf(stderr, "Bad realloc request format\n");
                break;
            }
            if (var < 0 || var >= NUM_VARS) {
                fprintf(stderr, "Variable out of range: %d\n", var);
                continue;
            }

            if (size == 0) {
                if (vars[var]) {
                    vars[var]->isFree = 1;
                    coalesce(vars[var]);
                    vars[var] = NULL;
                }
                continue;
            }

            if (vars[var]) {
                vars[var]->isFree = 1;
                coalesce(vars[var]);
                vars[var] = NULL;
            }

            struct memNode *freeNode = findFree(head, size, algo);
            if (!freeNode) {
                printf("ERROR: Out of Space\n");
                errorOutOfSpace = 1;
                break;
            }
            split(freeNode, size);
            vars[var] = freeNode;

        } else {
            fprintf(stderr, "Unknown operation code: %d\n", op);
            break;
        }
    }

    /* Always print lists, even on error */
    printList(head);
    printVars(vars);

    freeList(head);
    head = NULL;
    fclose(fp);

    if (errorOutOfSpace) return 2;
    return 0;
}