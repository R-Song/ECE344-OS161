#ifndef _VM_H_
#define _VM_H_

#include <machine/vm.h>
#include "opt-dumbvm.h"

/*
 * VM system-related definitions.
 * These functions act as the interface to the system's coremap and will be used in the kmalloc subroutines.
 * After vm_bootstrap is called, everything in the system is based off of virtual memory!
 */

/* Fault-type arguments to vm_fault() */
#define VM_FAULT_READ        0    /* A read was attempted */
#define VM_FAULT_WRITE       1    /* A write was attempted */
#define VM_FAULT_READONLY    2    /* A write to a readonly page was attempted*/

struct addrspace;
struct pte;

/* Initialization function */
void vm_bootstrap(void);

/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress);

int vm_readfault(struct addrspace *as, struct pte *faultentry, vaddr_t faultaddress, int is_pagefault, int is_stack, int is_swapped);
int vm_writefault(struct addrspace *as, struct pte *faultentry, vaddr_t faultaddress, int is_pagefault, int is_stack, int is_swapped);
int vm_stackfault(struct addrspace *as, vaddr_t faultaddress);
int vm_swapfault(struct pte *faultentry, vaddr_t faultaddress, int faulttype);

/* Allocate/free kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(int npages);
void    free_kpages(vaddr_t addr);

/* Allocate/free user pages */
void    alloc_upage(struct pte *entry);
void    free_upage(struct pte *entry);

/* Debug function */
#if OPT_DUMBVM
void region_dump(struct addrspace *as);
#endif

#endif /* _VM_H_ */
