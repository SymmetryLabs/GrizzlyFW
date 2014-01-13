/*
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 1.4
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
  Version History

  - v1.4:  Created  the   section  "Version  History".   Studied  real
           behaviour of actual applications (regular applications tend
           to require small memory blocks (less than 16 bytes) whereas
           TLSF is  optimised to  be used with  blocks larger  than 16
           bytes: Added special lists to deal with blocks smaller than
           16 bytes.
	   

  - v1.3: Change  of concept, now  the main TLSF structure  is created
          inside of  the beginning  of the block  instead of  being an
          static  structure, allowing  multiple TLSFs  working  at the
          same time.  TLSF now uses specific processor instructions to
          deal with bitmaps.  TLSF sanity functions added to find TLSF
          overflows.  The  TLSF code will not  be RTLinux-oriented any
          more.

  - v1.1 ...  v1.2: Many little  bugs fixed, code cleaned and splitted
	            in several files because of cosmetic requirements.
	            Starting     from    TLSF    v1.1,     MaRTE    OS
	            (marte.unican.es) uses  the TLSF allocator  as its
	            default memory allocator.

  - v0.1 ... v1.0: First implementations were for testing and research
                   purposes.  Basically TLSF is implemented to be used
                   by  RTLinux-GPL  (www.rtlinux-gpl.org),  so  it  is
                   RTLinux-oriented.
  
 */

#ifndef _THREE_LEVELS_SEGREGATE_FIT_MALLOC_H_
#define _THREE_LEVELS_SEGREGATE_FIT_MALLOC_H_
#include <stdlib.h>

/*------------------------------------------------------------------------*/
/******************************/
/*  CONFIGURATION PARAMETERS  */
/******************************/

// Following parameters allows to tune TLSF

// When SANITY_CHECK is set  TLSF functions execute different tests to
// check TLSF structure state, is something is wrong, an error message
// will be shown on the screen, but care must be taken because sanity 
// checks get worse TLSF performance

//#define SANITY_CHECK

/* TLSF_DEBUG_FUNCTIONS allows to use all debugging functions */
#define TLSF_DEBUG_FUNCTIONS

/* 
 * MAX_FL_INDEX defines the maximum first index which
 * will be used by TLSF. The maximum first index is
 * calculated in the init_memory_pool (size)
 * 
 * if (log2 (size) <= MAX_FL_INDEX) then
 *   max_fl_index := log2 (size);
 * else
 *   max_fl_index := MAX_FL_INDEX;
 * end if;
 *
 */

#define MAX_FL_INDEX 32 // TLSF default MAX_FL_INDEX is 16 MBytes

/* Standard functions name */
/* 
   Following macros define standard allocation/deallocation functions
 */

#define MALLOC_FUNCTION rtl_malloc
#define MALLOC_FUNCTION_EX rtl_malloc_ex

#define REALLOC_FUNCTION rtl_realloc
#define REALLOC_FUNCTION_EX rtl_realloc_ex

#define CALLOC_FUNCTION rtl_calloc
#define CALLOC_FUNCTION_EX rtl_calloc_ex

#define FREE_FUNCTION rtl_free
#define FREE_FUNCTION_EX rtl_free_ex


// Please, don't modify anything beyond this line
/*------------------------------------------------------------------------*/

#ifdef __RTL__

/* RTLinux module */

#include <linux/string.h>
#include <rtl.h>

#ifdef SANITY_CHECK
#define SANITY_PRINTF rtl_printf
#endif

#define PRINT_MSG rtl_printf
#define PRINT_DBG_C(message) rtl_printf(message)
#define PRINT_DBG_D(message) rtl_printf("%i", message);
//#define PRINT_DBG_F(message) rtl_printf("%f", message);
#define PRINT_DBG_H(message) rtl_printf("%x", (unsigned int) message);

#define INIT_THREAD_MUTEX()
#define THREAD_LOCK() __asm__ __volatile__ ("cli");
#define THREAD_UNLOCK() __asm__ __volatile__ ("sti");

#else

/* User space */

#include <string.h>
#include <stdio.h>

#ifdef SANITY_CHECK
#define SANITY_PRINTF printf
#endif

#define PRINT_MSG printf
#define PRINT_DBG_C(message) printf(message)
#define PRINT_DBG_D(message) printf("%i", message);
#define PRINT_DBG_F(message) printf("%f", message);
#define PRINT_DBG_H(message) printf("%x", (unsigned int) message);

#define INIT_THREAD_MUTEX()
#define THREAD_LOCK()
#define THREAD_UNLOCK()

#define __u8 unsigned char
#define __u16 unsigned short
#define __u32 unsigned int
#define __s8 char
#define __s16 short
#define __s32 int

#endif

extern char *main_buffer; // This buffer is associated with 
                          // a block of memory by the user

/*
 * associate  buffer allows  to indicate  to TLSF  that  one specific
 * buffer  must be  used by  default, this  allow to  the user  to use
 * malloc, free, calloc and realloc functions
 */

#define associate_buffer(ptr) main_buffer = (char *) ptr;

/*
 * max_sl_log2_index defines  the maximum  second index which  will be
 * used by TLSF.
 *
 * max_sl_log2_index allows  to the user to tune  the maximum internal
 * fragmentation, but  a high  max_sl_log2_index value will  cause big
 * TLSF structure.
 *
 * max_sl_log2_index  max. internal fragmentation (approximately)
 * -----------------  -------------------------------------------
 *     1                             25 %
 *     2                           12.5 %
 *     3                           6.25 %
 *     4                          3.125 %
 *     5                          1.563 %
 */

// max_size is in Kbytes
extern int init_memory_pool (int max_size, int max_sl_log2_index, 
			     size_t block_size, char *block_ptr);

extern void destroy_memory_pool (char *block_ptr);

/* see man malloc */
extern void *MALLOC_FUNCTION_EX (size_t size, char *block_ptr);

static inline void *MALLOC_FUNCTION (size_t size) {
  return (void *)MALLOC_FUNCTION_EX (size, main_buffer);
}   

/* see man realloc */
extern void *REALLOC_FUNCTION_EX (void *p, size_t new_len, char *block_ptr);

static inline void *REALLOC_FUNCTION (void *p, size_t new_len) {
  return (void *)REALLOC_FUNCTION_EX (p, new_len, main_buffer);
}

/* see man calloc */
extern void *CALLOC_FUNCTION_EX(size_t nelem, size_t elem_size, 
				char *block_ptr);

static inline void *CALLOC_FUNCTION(size_t nelem, 
				    size_t elem_size) {
  return (void *)CALLOC_FUNCTION_EX(nelem, elem_size, main_buffer);
}

/*
 * see man free
 *
 * free () is only guaranteed  to work if ptr is the address
 * of a block allocated by malloc() (and not yet freed).
 */

extern void FREE_FUNCTION_EX (void *ptr, char *block_ptr);

static inline void FREE_FUNCTION (void *ptr) {
  FREE_FUNCTION_EX (ptr, main_buffer);
}

#ifdef TLSF_DEBUG_FUNCTIONS
extern void free_blocks_status (char *block_ptr);
extern void structure_status (char *block_ptr);
extern void dump_memory_region (unsigned char *mem_ptr, unsigned int size);
#endif

#endif // #ifndef _THREE_LEVELS_SEGREGATE_FIT_MALLOC_H_
