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

#include "rtl_malloc.h"

#include "arch/bits.h"

#ifndef _TLSF_NONDEP_H_
#define _TLSF_NONDEP_H_

/*
 * The following  TAD will be a  double level indexed  array, the most
 * important thing is that the time  is bounded, the reason of this is
 * because TLSF has been designed to be used by real time programs.
 *
 *     First level       Second level
 *     it is indexed     it is indexed by 2**n+(2**n/m*index_number)
 *     by power of 2
 *                            0             1     m-1        m
 *                            ----> NULL   --> NULL          ----> NUL
 *                            |            |                 |
 *       -------         ---------------------...---------------------
 *  2**n |  n  |  -----> |2**n+(2**n/m*0)|...|   |...|2**n+(2**n/m*m)|
 *       |-----|         ---------------------...---------------------
 * 2**n-1| n-1 |  -----> ....                      |
 *       |-----|                                   --->NULL
 *        .....
 */

/* 
 * Some defaults values;
 * please, don't touch these macros
 */

#define TLSF_WORD_SIZE 4 //(sizeof(__u32))
#define LOG2_TLSF_WORD_SIZE 2 //(TLSF_fls(TLSF_WORD_SIZE)) // TLSF_WORD size
#define BYTES2TLSF_WORD_OFFSET_MASK 0x3 // to know the offset of a tranlation
#define TLSF_BLOCK_SIZE_MASK 0xFFFFFFFC

// IN TLSF_WORDS
#define MIN_LOG2_SIZE  3 // 32 bytes
#define MIN_SIZE 8 //(1 << MIN_LOG2_SIZE) // So the minimum block size is 
                                          //  32 Bytes

#define TLSF_WORDS2BYTES(x) ((x) << LOG2_TLSF_WORD_SIZE)
#define BYTES2TLSF_WORDS(x) ((x) >> LOG2_TLSF_WORD_SIZE)

/* 
 * The following structure defines the pointers used 
 * by the header to know the position of the free blocks 
 */

typedef struct free_ptr_struct {
  struct block_header_struct *prev;
  struct block_header_struct *next;

  /* 
   * first_index and second_index are used to store
   * mapping_function results, that's how we get some extra
   * nanoseconds 
   */
  __u8 first_index;
  __u8 second_index;
} free_ptr_t;

/*
 * USED_BLOCK must be used like mask with the magic number
 * i.e. 
 * if ((magic_number & USED_BLOCK) = USED_BLOCK) then
 *   return USED;
 * else
 *   return FREE
 * end if;
 */

#define USED_BLOCK 0x80000000
#define FREE_BLOCK ~USED_BLOCK //0x7FFFFFFF

#define LAST_BLOCK 0x40000000
#define NOT_LAST_BLOCK ~LAST_BLOCK //0xBFFFFFFF

#define IS_USED_BLOCK(x) ((x -> size & USED_BLOCK) == USED_BLOCK)
#define IS_LAST_BLOCK(x) ((x -> size & LAST_BLOCK) == LAST_BLOCK)
#define GET_BLOCK_SIZE(x) (x -> size & FREE_BLOCK & NOT_LAST_BLOCK)
#define SET_USED_BLOCK(x) (x -> size |= USED_BLOCK)
#define SET_FREE_BLOCK(x) (x -> size &= FREE_BLOCK)
#define SET_LAST_BLOCK(x) (x -> size |= LAST_BLOCK)
#define SET_NOT_LAST_BLOCK(x) (x -> size &= NOT_LAST_BLOCK)

#define MAGIC_NUMBER 0x2A59FA59

typedef struct block_header_struct {
#ifdef SANITY_CHECK
  __u32 mw;
#endif
  /* 
   * size codifies the block size in TLSF_BYTES and it also codifies if 
   * the block is used or free 
   */
  
  __u32 size;
  
  /* The following pointer points to the previous physical block */
  struct block_header_struct *prev_phys_block;

  union {
    struct free_ptr_struct free_ptr;
    __u8 buffer[sizeof(struct free_ptr_struct)];
  } ptr;
  
} block_header_t;

/* first level index array */
typedef struct fl_array_struct {
  /* ptr is pointer to next level */
  block_header_t **sl_array;
  
  /* bitmapSL is the second level bitmap */
  __u32 bitmapSL;
} fl_array_t;

/* 
 * When the user calls init_memory_pool, he must give a block of memory
 * block, this block will be used to store the TLSF structure
 */

typedef struct TLSF_struct {
  __u32 magic_number;
  /* 
   * max_fl_index, max_sl_index and max_sl_log2_index
   * must be __u8 but the compiler assigns 32 bits by efficiency, 
   * we also do that
   */
  __u32 max_fl_index;
  __u32 TLSF_max_struct_size;
  __u32 max_sl_index;
  __u32 max_sl_log2_index;
  
  /* bitmapFL is the bitmap of the first level */
  __u32 bitmapFL;

  /* 
   * each new  block added with add_new_block function  will be linked
   * through a  linked free list following_non_cont_bh  points to the
   * head
   */
  __u32 *following_non_cont_bh;

  // Starting  from version  1.4, TLSF  supports several  special list
  // which only  point blocks  of 8,  12, 16, 20,  24, 28  bytes. This
  // strategy has been  adopted since there exist a  large majority of
  // applications which use these sizes, and TLSF does not perform too
  // well with them.   (A cheat? it isn't, it is just  a change in the
  // used strategy)
  
  // It  is important to  note that  the size  of a  block_header_t is
  // bigger than the block which is going to contain the structure
  // so it is quite important not to use the fields either "first_index"
  // or "second_index" since any change will overwrite other memory node
  // breaking the global behaviour of the algorithm
  __u32 small_blocks_bitmap;
  block_header_t *small_blocks_array [6]; // 8, 12, 16, 20, 24, 28  
  
  /* 
   * fl_array will be our first level array,
   * it will be an array of [max_fl_index] elements
   */
  fl_array_t *fl_array;
} TLSF_t;

/*
 * header_overhead has size of blocks_header - 2 * ptr to next free block
 */
extern __s32 beg_header_overhead;
extern char *main_buffer;

#define TLSF__set_bit(num, mem) mem |= (1 << num)
#define TLSF__clear_bit(num, mem) mem &= ~(1 << num)

/*
 * log2size ()  returns cell  of log2 (len)  it does a  search between
 * MIN_SIZE and MAX_SIZE values in order to find the log2 of the size.
 * Theorically  we have obtained  that this  method is  more efficient
 * than the asm instruction which does the same operation
 */

static inline __s32 log2size (size_t size, size_t *new_size) {
  
  __s32 i;

  /* Our objective is to look for the first 1 bit */
  i = TLSF_fls(size);
  
  *new_size = (1 << i);
  
  return i;
}

/*
 * mapping_function () returns first and second level index
 *
 * first level index function is:
 * fl = log2 (size)
 *
 * and second level index function is:
 * sl = (size - 2**(log2size (size))) / (log2 (size) - log2 (MAX_SL_INDEX))
 *
 */

static inline __s32 mapping_function (size_t size, __s32 *fl, __s32 *sl, 
				      size_t *new_size, TLSF_t *ptr_TLSF){
  __s32 aux, pow_size;
   
  /* This is a new way to calculate first level index 
     and second level index, it is quicker than older one */
  *fl = log2size (size, &pow_size);

  aux = (*fl - ptr_TLSF -> max_sl_log2_index);
  aux = (aux > 0)? aux:0;

  *new_size = (((size >> aux) + 1 ) << aux);

  *sl = ((size ^ pow_size) >> aux);

  return 0;
}

/* External C function definitions */
/*
extern void *memset(void *s, int c, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
*/

#ifdef SANITY_CHECK
#include "TLSF_sanity.h"
#endif
#endif
