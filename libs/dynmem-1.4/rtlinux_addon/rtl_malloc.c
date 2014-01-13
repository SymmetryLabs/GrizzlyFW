/*
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 1.3
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 *
 * Thanks to Ismael Ripoll for his suggestions and reviews
 *
 * Copyright (C) April 2004 UPVLC, OCERA Consortium
 * 
 * This code is released using a dual license strategy: GPL/LGPL
 * You can choose the license that better fits your requirements.
 *
 * Released under the terms of the GNU General Public License Version 2.0
 * Released under the terms of the GNU Lesser General Public License Version 2.1
 *
 */
/*
   This file contains wrappers to use TLSF from RTLinux 

   - init_malloc: 
     
   It creates an empty buffer  of "max_size" Kbytes which will be used
   through rtl_malloc, rtl_free, rtl_realloc by RTLinux applications.
   
   Since this function uses vmalloc, a non real-time safe function, it
   must be called in a secure way (from a init_module function)
   

   - cleanup_malloc:
   
   It destroy a previously  allocated buffer using the kernel function
   "vfree"

   Like vmalloc, vfree is not a  real-time safe function either so it can only
   be executed in a secure way (i.e. cleanup_module)
   
 */

#include <rtl.h>

#include <linux/vmalloc.h>

#include "rtl_malloc.h"
#include "TLSF_nondep.h"

static unsigned char *ptr_mem;

int init_malloc(int max_size){
  // First of all we reserve the memory that will be used later
  ptr_mem = (unsigned char *) vmalloc (max_size * 1024);
  if (!ptr_mem) {
    rtl_printf ("rtl_malloc PANIC: Error reserving memory!!");
    return -1;
  }
  
  memset(ptr_mem, 0x00, max_size * 1024);
  if (init_memory_pool (0, 5, max_size, ptr_mem) < 0) return -1;
  // associate buffer allows using directly rtl_malloc and rtl_free
  // functions instead of rtl_malloc_ex and rtl_free_ex
  associate_buffer (ptr_mem);
  return 0;
}

void cleanup_malloc(void){
  destroy_memory_pool(ptr_mem);
  vfree (ptr_mem); 
}
