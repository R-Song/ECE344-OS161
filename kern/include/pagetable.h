
/*
 * Defintion of the pagetable
 *  
 * The purpose of a page table is to translate virtual addresses to physical addresses.
 * Each addrspace should have its own page table. 
 * 
 */

#ifndef _PAGETABLE_H_
#define _PAGETABLE_H_

#include <permissions.h>
#include <machine/vm.h>
#include "opt-twolevelpagetable.h"



/******************************************************************************
 * Definition and operation on page table entries (pte)
 * right now just has physical mapping, but will need more fields later...
 ******************************************************************************/
struct pte {
	paddr_t ppageaddr;
    /* Tentative fields to add to support copy on write */
    struct semaphore *pte_mutex; /* Mutual exclusion for this page */
    int num_users;               /* How many users are reading from this page? */
    int dirty;                   /* Is this page safe to write to */
    /* permissions */
    permissions_t permissions;
};

/* create and destroy a pte */
struct pte *pte_init();
void pte_destroy(struct pte *entry); 

/* Copy a pte entry */
void pte_copy(struct pte *src, struct pte *dest);



/***********************************************
 * Different implementations of the page table
 ***********************************************/
#if !OPT_TWOLEVELPAGETABLE

/*
 * A slower, but more space efficient implementation of the page table.
 * The layout of the page table is a linked list with each node holding an array.
 * We map the top 16 bits using a linked list, then the bottom 4 bits using an array.
 * These numbers are hardcoded and can be adjusted and experiemented with.
 * With this method, a page table uses substantially less memory. And for the 
 * common case, increases the maximum number of memory accesses from 3 to 6
 */

struct pte_container {
    u_int32_t first_idx;
    struct pte **pte_array;
    struct pte_container *next;
};

#define PT_FIRST_INDEX_OFFSET            16
#define PT_SECOND_INDEX_OFFSET           12
#define PT_VADDR_TO_FIRST_INDEX(vaddr)   (vaddr>>PT_FIRST_INDEX_OFFSET)
#define PT_VADDR_TO_SECOND_INDEX(vaddr)  ( (vaddr>>PT_SECOND_INDEX_OFFSET) & 0x0000000f )
#define PT_PTE_ARRAY_NUM_ENTRIES         16
#define PT_INDEX_TO_VADDR(idx1, idx2)    ( (idx1<<PT_FIRST_INDEX_OFFSET) | (idx2<<PT_SECOND_INDEX_OFFSET) )


typedef struct pte_container* pagetable_t;


#else

/*
 * This implements a two level page table.
 * 
 * In this specific case, there are 2^19 user level pages that we need to potentially provide a mapping for.
 * The first page table will have 2^9 (512) entries, each with a pointer to another table with 2^10 (1024) entries.
 * This covers all 19 bits we need to describe the virtual page number. The second layer of tables is created on demand.
 * Each of the entries in one of the tables with 512 entries is a page table entry, with a ton of useful information about the page.
 * 
 * This shouldnt be used until we get a better page table implementation going.
 */

/*
 * Machine dependant stuff
 * Hardcoded sizes of the first and second layer. Really depends on the machine.
 * Here, the first and second layer cover 19 bits, which is enough to index from MIPS_KUSEG to MIPS_KSEG0
 */
#define PT_FIRST_LAYER_SIZE 512
#define PT_SECOND_LAYER_SIZE 1024
#define PT_SECOND_LAYER_OFFSET 10
#define PT_SECOND_LAYER_MASK 0x000003ff     /* bottom 10 bits */

u_int32_t pt_vaddr_to_first_index(vaddr_t addr);
u_int32_t pt_vaddr_to_second_index(vaddr_t addr);
vaddr_t idx_to_vaddr(u_int32_t first_idx, u_int32_t second_idx);

/* pagetable definition */
typedef struct pte*** pagetable_t;     


#endif /* OPT_TWOLEVELPAGETABLE */



/************************************************************************************
 * The higher level functions for the page table should be he same for the 
 * two page table implementations, as they're used throughout the rest of the kernel
 ************************************************************************************/

/* initialize a new page table */
pagetable_t pt_init();

/* add entry into the page table */
int pt_add(pagetable_t pt, vaddr_t vaddr, struct pte *entry);

/* get pte entry */
struct pte *pt_get(pagetable_t pt, vaddr_t vaddr);

/* get the next populated vaddr */
vaddr_t pt_getnext(pagetable_t pt, vaddr_t vaddr);

/* copy a page table */
int pt_copy(pagetable_t src, pagetable_t dest);

/* remove entry from the page table */
void pt_remove(pagetable_t pt, vaddr_t vaddr);

/* destroy page table */
void pt_destroy(pagetable_t pt);

/* dump all contents of pagetable to console */
void pt_dump(pagetable_t pt);



#endif /* _PAGETABLE_H_ */
