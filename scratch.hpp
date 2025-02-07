#ifndef _SCRATCH_HPP
#define _SCRATCH_HPP

#include <stdio.h>

void* scratch_alloc(unsigned int size);
void scratch_reset(void) ;

#define scratch_alloc_struct(count, type) (type*) scratch_alloc(count * sizeof(type))


#endif