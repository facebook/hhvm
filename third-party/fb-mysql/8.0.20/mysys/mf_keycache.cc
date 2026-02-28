/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/mf_keycache.cc
  These functions handle keyblock caching for ISAM and MyISAM tables.

  One cache can handle many files.
  It must contain buffers of the same blocksize.
  init_key_cache() should be used to init cache handler.

  The free list (free_block_list) is a stack like structure.
  When a block is freed by free_block(), it is pushed onto the stack.
  When a new block is required it is first tried to pop one from the stack.
  If the stack is empty, it is tried to get a never-used block from the pool.
  If this is empty too, then a block is taken from the LRU ring, flushing it
  to disk, if neccessary. This is handled in find_key_block().
  With the new free list, the blocks can have three temperatures:
  hot, warm and cold (which is free). This is remembered in the block header
  by the enum BLOCK_TEMPERATURE temperature variable. Remembering the
  temperature is neccessary to correctly count the number of warm blocks,
  which is required to decide when blocks are allowed to become hot. Whenever
  a block is inserted to another (sub-)chain, we take the old and new
  temperature into account to decide if we got one more or less warm block.
  blocks_unused is the sum of never used blocks in the pool and of currently
  free blocks. blocks_used is the number of blocks fetched from the pool and
  as such gives the maximum number of in-use blocks at any time.
*/

/*
  Key Cache Locking
  =================

  All key cache locking is done with a single mutex per key cache:
  keycache->cache_lock. This mutex is locked almost all the time
  when executing code in this file (mf_keycache.c).
  However it is released for I/O and some copy operations.

  The cache_lock is also released when waiting for some event. Waiting
  and signalling is done via condition variables. In most cases the
  thread waits on its thread->suspend condition variable. Every thread
  has a my_thread_var structure, which contains this variable and a
  '*next' and '**prev' pointer. These pointers are used to insert the
  thread into a wait queue.

  A thread can wait for one block and thus be in one wait queue at a
  time only.

  Before starting to wait on its condition variable with
  mysql_cond_wait(), the thread enters itself to a specific wait queue
  with link_into_queue() (double linked with '*next' + '**prev') or
  wait_on_queue() (single linked with '*next').

  Another thread, when releasing a resource, looks up the waiting thread
  in the related wait queue. It sends a signal with
  mysql_cond_signal() to the waiting thread.

  NOTE: Depending on the particular wait situation, either the sending
  thread removes the waiting thread from the wait queue with
  unlink_from_queue() or release_whole_queue() respectively, or the waiting
  thread removes itself.

  There is one exception from this locking scheme when one thread wants
  to reuse a block for some other address. This works by first marking
  the block reserved (status= BLOCK_IN_SWITCH) and then waiting for all
  threads that are reading the block to finish. Each block has a
  reference to a condition variable (condvar). It holds a reference to
  the thread->suspend condition variable for the waiting thread (if such
  a thread exists). When that thread is signaled, the reference is
  cleared. The number of readers of a block is registered in
  block->hash_link->requests. See wait_for_readers() / remove_reader()
  for details. This is similar to the above, but it clearly means that
  only one thread can wait for a particular block. There is no queue in
  this case. Strangely enough block->convar is used for waiting for the
  assigned hash_link only. More precisely it is used to wait for all
  requests to be unregistered from the assigned hash_link.

  The resize_queue serves two purposes:
  1. Threads that want to do a resize wait there if in_resize is set.
     This is not used in the server. The server refuses a second resize
     request if one is already active. keycache->in_init is used for the
     synchronization. See set_var.cc.
  2. Threads that want to access blocks during resize wait here during
     the re-initialization phase.
  When the resize is done, all threads on the queue are signalled.
  Hypothetical resizers can compete for resizing, and read/write
  requests will restart to request blocks from the freshly resized
  cache. If the cache has been resized too small, it is disabled and
  'can_be_used' is false. In this case read/write requests bypass the
  cache. Since they increment and decrement 'cnt_for_resize_op', the
  next resizer can wait on the queue 'waiting_for_resize_cnt' until all
  I/O finished.
*/

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>

#include "keycache.h"
#include "my_bit.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_pointer_arithmetic.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"
#include "template_utils.h"
#include "thr_mutex.h"

#define STRUCT_PTR(TYPE, MEMBER, a) (TYPE *)((char *)(a)-offsetof(TYPE, MEMBER))

/* types of condition variables */
#define COND_FOR_REQUESTED 0
#define COND_FOR_SAVED 1

typedef mysql_cond_t KEYCACHE_CONDVAR;

/* descriptor of the page in the key cache block buffer */
struct KEYCACHE_PAGE {
  int file;         /* file to which the page belongs to  */
  my_off_t filepos; /* position of the page in the file   */
};

/* element in the chain of a hash table bucket */
struct HASH_LINK {
  HASH_LINK *next, **prev; /* to connect links in the same bucket  */
  BLOCK_LINK *block;       /* reference to the block for the page: */
  File file;               /* from such a file                     */
  my_off_t diskpos;        /* with such an offset                  */
  uint requests;           /* number of requests for the page      */
};

/* simple states of a block */
#define BLOCK_ERROR 1           /* an error occurred when performing file i/o */
#define BLOCK_READ 2            /* file block is in the block buffer         */
#define BLOCK_IN_SWITCH 4       /* block is preparing to read new page       */
#define BLOCK_REASSIGNED 8      /* blk does not accept requests for old page */
#define BLOCK_IN_FLUSH 16       /* block is selected for flush               */
#define BLOCK_CHANGED 32        /* block buffer contains a dirty page        */
#define BLOCK_IN_USE 64         /* block is not free                         */
#define BLOCK_IN_EVICTION 128   /* block is selected for eviction            */
#define BLOCK_IN_FLUSHWRITE 256 /* block is in write to file */
#define BLOCK_FOR_UPDATE 512    /* block is selected for buffer modification */

/* page status, returned by find_key_block */
#define PAGE_READ 0
#define PAGE_TO_BE_READ 1
#define PAGE_WAIT_TO_BE_READ 2

/* block temperature determines in which (sub-)chain the block currently is */
enum BLOCK_TEMPERATURE { BLOCK_COLD /*free*/, BLOCK_WARM, BLOCK_HOT };

/* key cache block */
struct BLOCK_LINK {
  BLOCK_LINK
  *next_used, **prev_used; /* to connect links in the LRU chain (ring)   */
  BLOCK_LINK
  *next_changed, **prev_changed; /* for lists of file dirty/clean blocks   */
  HASH_LINK *hash_link;          /* backward ptr to referring hash_link     */
  KEYCACHE_WQUEUE wqueue[2]; /* queues on waiting requests for new/old pages */
  uint requests; /* number of requests for the block                */
  uchar *buffer; /* buffer for the block page                       */
  uint offset;   /* beginning of modified data in the buffer        */
  uint length;   /* end of data in the buffer                       */
  uint status;   /* state of the block                              */
  enum BLOCK_TEMPERATURE temperature; /* block temperature: cold, warm, hot */
  uint hits_left;          /* number of hits left until promotion             */
  ulonglong last_hit_time; /* timestamp of the last hit                      */
  KEYCACHE_CONDVAR *condvar; /* condition variable for 'no readers' event    */
};

KEY_CACHE dflt_key_cache_var;
KEY_CACHE *dflt_key_cache = &dflt_key_cache_var;

#define FLUSH_CACHE 2000 /* sort this many blocks at once */

static void change_key_cache_param(KEY_CACHE *keycache,
                                   ulonglong division_limit,
                                   ulonglong age_threshold);
static int flush_all_key_blocks(KEY_CACHE *keycache,
                                st_keycache_thread_var *thread_var);

static void wait_on_queue(KEYCACHE_WQUEUE *wqueue, mysql_mutex_t *mutex,
                          st_keycache_thread_var *thread);
static void release_whole_queue(KEYCACHE_WQUEUE *wqueue);

static void free_block(KEY_CACHE *keycache, st_keycache_thread_var *thread_var,
                       BLOCK_LINK *block);

#define KEYCACHE_HASH(f, pos)                                       \
  (((ulong)((pos) / keycache->key_cache_block_size) + (ulong)(f)) & \
   (keycache->hash_entries - 1))
#define FILE_HASH(f) ((uint)(f) & (CHANGED_BLOCKS_HASH - 1))

#define BLOCK_NUMBER(b) \
  ((uint)(((char *)(b) - (char *)keycache->block_root) / sizeof(BLOCK_LINK)))
#ifdef KEYCACHE_TIMEOUT
#define HASH_LINK_NUMBER(h) \
  ((uint)(((char *)(h) - (char *)keycache->hash_link_root) / sizeof(HASH_LINK)))
#endif

#if !defined(DBUG_OFF)
static int fail_block(BLOCK_LINK *block);
static int fail_hlink(HASH_LINK *hlink);
static int cache_empty(KEY_CACHE *keycache);
#endif

static inline uint next_power(uint value) {
  return (uint)my_round_up_to_next_power((uint32)value) << 1;
}

/*
  Initialize a key cache

  SYNOPSIS
    init_key_cache()
    keycache			pointer to a key cache data structure
    key_cache_block_size	size of blocks to keep cached data
    use_mem                 	total memory to use for the key cache
    division_limit		division limit (may be zero)
    age_threshold		age threshold (may be zero)

  RETURN VALUE
    number of blocks in the key cache, if successful,
    0 - otherwise.

  NOTES.
    if keycache->key_cache_inited != 0 we assume that the key cache
    is already initialized.  This is for now used by myisamchk, but shouldn't
    be something that a program should rely on!

    It's assumed that no two threads call this function simultaneously
    referring to the same key cache handle.

*/

int init_key_cache(KEY_CACHE *keycache, ulonglong key_cache_block_size,
                   size_t use_mem, ulonglong division_limit,
                   ulonglong age_threshold) {
  ulong blocks, hash_links;
  size_t length;
  int error;
  DBUG_TRACE;
  DBUG_ASSERT(key_cache_block_size >= 512);

  if (keycache->key_cache_inited && keycache->disk_blocks > 0) {
    DBUG_PRINT("warning", ("key cache already in use"));
    return 0;
  }

  keycache->global_cache_w_requests = keycache->global_cache_r_requests = 0;
  keycache->global_cache_read = keycache->global_cache_write = 0;
  keycache->disk_blocks = -1;
  if (!keycache->key_cache_inited) {
    keycache->key_cache_inited = true;
    /*
      Initialize these variables once only.
      Their value must survive re-initialization during resizing.
    */
    keycache->in_resize = false;
    keycache->resize_in_flush = false;
    keycache->cnt_for_resize_op = 0;
    keycache->waiting_for_resize_cnt.last_thread = nullptr;
    keycache->in_init = false;
    mysql_mutex_init(key_KEY_CACHE_cache_lock, &keycache->cache_lock,
                     MY_MUTEX_INIT_FAST);
    keycache->resize_queue.last_thread = nullptr;
  }

  keycache->key_cache_mem_size = use_mem;
  keycache->key_cache_block_size = (uint)key_cache_block_size;
  DBUG_PRINT("info", ("key_cache_block_size: %llu", key_cache_block_size));

  blocks =
      (ulong)(use_mem / (sizeof(BLOCK_LINK) + 2 * sizeof(HASH_LINK) +
                         sizeof(HASH_LINK *) * 5 / 4 + key_cache_block_size));
  /* It doesn't make sense to have too few blocks (less than 8) */
  if (blocks >= 8) {
    for (;;) {
      /* Set my_hash_entries to the next bigger 2 power */
      if ((keycache->hash_entries = next_power(blocks)) < blocks * 5 / 4)
        keycache->hash_entries <<= 1;
      hash_links = 2 * blocks;
      while ((length =
                  (ALIGN_SIZE(blocks * sizeof(BLOCK_LINK)) +
                   ALIGN_SIZE(hash_links * sizeof(HASH_LINK)) +
                   ALIGN_SIZE(sizeof(HASH_LINK *) * keycache->hash_entries))) +
                 ((size_t)blocks * keycache->key_cache_block_size) >
             use_mem)
        blocks--;
      /* Allocate memory for cache page buffers */
      if ((keycache->block_mem = static_cast<uchar *>(my_malloc(
               key_memory_KEY_CACHE,
               (size_t)blocks * keycache->key_cache_block_size, MYF(0))))) {
        /*
          Allocate memory for blocks, hash_links and hash entries;
          For each block 2 hash links are allocated
        */
        if ((keycache->block_root =
                 (BLOCK_LINK *)my_malloc(key_memory_KEY_CACHE, length, MYF(0))))
          break;
        my_free(keycache->block_mem);
        keycache->block_mem = nullptr;
      }
      if (blocks < 8) {
        set_my_errno(ENOMEM);
        my_error(EE_OUTOFMEMORY, MYF(ME_FATALERROR),
                 blocks * keycache->key_cache_block_size);
        goto err;
      }
      blocks = blocks / 4 * 3;
    }
    keycache->blocks_unused = blocks;
    keycache->disk_blocks = (int)blocks;
    keycache->hash_links = hash_links;
    keycache->hash_root =
        (HASH_LINK **)((char *)keycache->block_root +
                       ALIGN_SIZE(blocks * sizeof(BLOCK_LINK)));
    keycache->hash_link_root =
        (HASH_LINK *)((char *)keycache->hash_root +
                      ALIGN_SIZE(
                          (sizeof(HASH_LINK *) * keycache->hash_entries)));
    memset(keycache->block_root, 0, keycache->disk_blocks * sizeof(BLOCK_LINK));
    memset(keycache->hash_root, 0,
           keycache->hash_entries * sizeof(HASH_LINK *));
    memset(keycache->hash_link_root, 0,
           keycache->hash_links * sizeof(HASH_LINK));
    keycache->hash_links_used = 0;
    keycache->free_hash_list = nullptr;
    keycache->blocks_used = keycache->blocks_changed = 0;

    keycache->global_blocks_changed = 0;
    keycache->blocks_available = 0; /* For debugging */

    /* The LRU chain is empty after initialization */
    keycache->used_last = nullptr;
    keycache->used_ins = nullptr;
    keycache->free_block_list = nullptr;
    keycache->keycache_time = 0;
    keycache->warm_blocks = 0;
    keycache->min_warm_blocks =
        (division_limit ? blocks * division_limit / 100 + 1 : blocks);
    keycache->age_threshold =
        (age_threshold ? blocks * age_threshold / 100 : blocks);

    keycache->can_be_used = true;

    keycache->waiting_for_hash_link.last_thread = nullptr;
    keycache->waiting_for_block.last_thread = nullptr;
    DBUG_PRINT("exit", ("disk_blocks: %d  block_root: %p  hash_entries: %d\
 hash_root: %p  hash_links: %d  hash_link_root: %p",
                        keycache->disk_blocks, keycache->block_root,
                        keycache->hash_entries, keycache->hash_root,
                        keycache->hash_links, keycache->hash_link_root));
    memset(keycache->changed_blocks, 0,
           sizeof(keycache->changed_blocks[0]) * CHANGED_BLOCKS_HASH);
    memset(keycache->file_blocks, 0,
           sizeof(keycache->file_blocks[0]) * CHANGED_BLOCKS_HASH);
  } else {
    /* key_buffer_size is specified too small. Disable the cache. */
    keycache->can_be_used = false;
  }

  keycache->blocks = keycache->disk_blocks > 0 ? keycache->disk_blocks : 0;
  return (int)keycache->disk_blocks;

err:
  error = my_errno();
  keycache->disk_blocks = 0;
  keycache->blocks = 0;
  if (keycache->block_mem) {
    my_free((uchar *)keycache->block_mem);
    keycache->block_mem = nullptr;
  }
  if (keycache->block_root) {
    my_free(keycache->block_root);
    keycache->block_root = nullptr;
  }
  set_my_errno(error);
  keycache->can_be_used = false;
  return 0;
}

/*
  Resize a key cache

  SYNOPSIS
    resize_key_cache()
    keycache     	        pointer to a key cache data structure
    thread_var                  pointer to thread specific variables
    key_cache_block_size        size of blocks to keep cached data
    use_mem			total memory to use for the new key cache
    division_limit		new division limit (if not zero)
    age_threshold		new age threshold (if not zero)

  RETURN VALUE
    number of blocks in the key cache, if successful,
    0 - otherwise.

  NOTES.
    The function first compares the memory size and the block size parameters
    with the key cache values.

    If they differ the function free the the memory allocated for the
    old key cache blocks by calling the end_key_cache function and
    then rebuilds the key cache with new blocks by calling
    init_key_cache.

    The function starts the operation only when all other threads
    performing operations with the key cache let her to proceed
    (when cnt_for_resize=0).
*/

int resize_key_cache(KEY_CACHE *keycache, st_keycache_thread_var *thread_var,
                     ulonglong key_cache_block_size, size_t use_mem,
                     ulonglong division_limit, ulonglong age_threshold) {
  int blocks;
  DBUG_TRACE;

  if (!keycache->key_cache_inited) return keycache->disk_blocks;

  if (key_cache_block_size == keycache->key_cache_block_size &&
      use_mem == keycache->key_cache_mem_size) {
    change_key_cache_param(keycache, division_limit, age_threshold);
    return keycache->disk_blocks;
  }

  mysql_mutex_lock(&keycache->cache_lock);

  /*
    We may need to wait for another thread which is doing a resize
    already. This cannot happen in the MySQL server though. It allows
    one resizer only. In set_var.cc keycache->in_init is used to block
    multiple attempts.
  */
  while (keycache->in_resize) {
    /* purecov: begin inspected */
    wait_on_queue(&keycache->resize_queue, &keycache->cache_lock, thread_var);
    /* purecov: end */
  }

  /*
    Mark the operation in progress. This blocks other threads from doing
    a resize in parallel. It prohibits new blocks to enter the cache.
    Read/write requests can bypass the cache during the flush phase.
  */
  keycache->in_resize = true;

  /* Need to flush only if keycache is enabled. */
  if (keycache->can_be_used) {
    /* Start the flush phase. */
    keycache->resize_in_flush = true;

    if (flush_all_key_blocks(keycache, thread_var)) {
      /* TODO: if this happens, we should write a warning in the log file ! */
      keycache->resize_in_flush = false;
      blocks = 0;
      keycache->can_be_used = false;
      goto finish;
    }
    DBUG_ASSERT(cache_empty(keycache));

    /* End the flush phase. */
    keycache->resize_in_flush = false;
  }

  /*
    Some direct read/write operations (bypassing the cache) may still be
    unfinished. Wait until they are done. If the key cache can be used,
    direct I/O is done in increments of key_cache_block_size. That is,
    every block is checked if it is in the cache. We need to wait for
    pending I/O before re-initializing the cache, because we may change
    the block size. Otherwise they could check for blocks at file
    positions where the new block division has none. We do also want to
    wait for I/O done when (if) the cache was disabled. It must not
    run in parallel with normal cache operation.
  */
  while (keycache->cnt_for_resize_op)
    wait_on_queue(&keycache->waiting_for_resize_cnt, &keycache->cache_lock,
                  thread_var);

  /*
    Free old cache structures, allocate new structures, and initialize
    them. Note that the cache_lock mutex and the resize_queue are left
    untouched. We do not lose the cache_lock and will release it only at
    the end of this function.
  */
  end_key_cache(keycache, false); /* Don't free mutex */
  /* The following will work even if use_mem is 0 */
  blocks = init_key_cache(keycache, key_cache_block_size, use_mem,
                          division_limit, age_threshold);

finish:
  /*
    Mark the resize finished. This allows other threads to start a
    resize or to request new cache blocks.
  */
  keycache->in_resize = false;

  /* Signal waiting threads. */
  release_whole_queue(&keycache->resize_queue);

  mysql_mutex_unlock(&keycache->cache_lock);
  return blocks;
}

/*
  Increment counter blocking resize key cache operation
*/
static inline void inc_counter_for_resize_op(KEY_CACHE *keycache) {
  keycache->cnt_for_resize_op++;
}

/*
  Decrement counter blocking resize key cache operation;
  Signal the operation to proceed when counter becomes equal zero
*/
static inline void dec_counter_for_resize_op(KEY_CACHE *keycache) {
  if (!--keycache->cnt_for_resize_op)
    release_whole_queue(&keycache->waiting_for_resize_cnt);
}

/*
  Change the key cache parameters

  SYNOPSIS
    change_key_cache_param()
    keycache			pointer to a key cache data structure
    division_limit		new division limit (if not zero)
    age_threshold		new age threshold (if not zero)

  RETURN VALUE
    none

  NOTES.
    Presently the function resets the key cache parameters
    concerning midpoint insertion strategy - division_limit and
    age_threshold.
*/

static void change_key_cache_param(KEY_CACHE *keycache,
                                   ulonglong division_limit,
                                   ulonglong age_threshold) {
  DBUG_TRACE;

  mysql_mutex_lock(&keycache->cache_lock);
  if (division_limit)
    keycache->min_warm_blocks =
        (keycache->disk_blocks * division_limit / 100 + 1);
  if (age_threshold)
    keycache->age_threshold = (keycache->disk_blocks * age_threshold / 100);
  mysql_mutex_unlock(&keycache->cache_lock);
}

/*
  Remove key_cache from memory

  SYNOPSIS
    end_key_cache()
    keycache		key cache handle
    cleanup		Complete free (Free also mutex for key cache)

  RETURN VALUE
    none
*/

void end_key_cache(KEY_CACHE *keycache, bool cleanup) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("key_cache: %p", keycache));

  if (!keycache->key_cache_inited) return;

  if (keycache->disk_blocks > 0) {
    if (keycache->block_mem) {
      my_free((uchar *)keycache->block_mem);
      keycache->block_mem = nullptr;
      my_free(keycache->block_root);
      keycache->block_root = nullptr;
    }
    keycache->disk_blocks = -1;
    /* Reset blocks_changed to be safe if flush_all_key_blocks is called */
    keycache->blocks_changed = 0;
  }

  DBUG_PRINT("status", ("used: %lu  changed: %lu  w_requests: %lu  "
                        "writes: %lu  r_requests: %lu  reads: %lu",
                        keycache->blocks_used, keycache->global_blocks_changed,
                        (ulong)keycache->global_cache_w_requests,
                        (ulong)keycache->global_cache_write,
                        (ulong)keycache->global_cache_r_requests,
                        (ulong)keycache->global_cache_read));

  /*
    Reset these values to be able to detect a disabled key cache.
    See Bug#44068 (RESTORE can disable the MyISAM Key Cache).
  */
  keycache->blocks_used = 0;
  keycache->blocks_unused = 0;

  if (cleanup) {
    mysql_mutex_destroy(&keycache->cache_lock);
    keycache->key_cache_inited = keycache->can_be_used = false;
  }
} /* end_key_cache */

/**
  Link a thread into double-linked queue of waiting threads.

  @param wqueue   pointer to the queue structure
  @param thread   pointer to the keycache variables for the
                  thread to be added to the queue

  Queue is represented by a circular list of the keycache variable structures.
  Since each thread has its own keycache variables, this is equal to a list
  of threads. The list is double-linked of the type (**prev,*next), accessed by
  a pointer to the last element.
*/

static void link_into_queue(KEYCACHE_WQUEUE *wqueue,
                            st_keycache_thread_var *thread) {
  st_keycache_thread_var *last;

  DBUG_ASSERT(!thread->next && !thread->prev);
  if (!(last = wqueue->last_thread)) {
    /* Queue is empty */
    thread->next = thread;
    thread->prev = &thread->next;
  } else {
    thread->prev = last->next->prev;
    last->next->prev = &thread->next;
    thread->next = last->next;
    last->next = thread;
  }
  wqueue->last_thread = thread;
}

/**
  Unlink a thread from double-linked queue of waiting threads

  @param wqueue   pointer to the queue structure
  @param thread   pointer to the keycache variables for the
                  thread to be removed to the queue

  @note See link_into_queue
*/

static void unlink_from_queue(KEYCACHE_WQUEUE *wqueue,
                              st_keycache_thread_var *thread) {
  DBUG_ASSERT(thread->next && thread->prev);
  if (thread->next == thread) /* The queue contains only one member */
    wqueue->last_thread = nullptr;
  else {
    thread->next->prev = thread->prev;
    *thread->prev = thread->next;
    if (wqueue->last_thread == thread)
      wqueue->last_thread =
          STRUCT_PTR(st_keycache_thread_var, next, thread->prev);
  }
  thread->next = nullptr;
#if !defined(DBUG_OFF)
  /*
    This makes it easier to see it's not in a chain during debugging.
    And some DBUG_ASSERT() rely on it.
  */
  thread->prev = nullptr;
#endif
}

/*
  Add a thread to single-linked queue of waiting threads

  SYNOPSIS
    wait_on_queue()
      wqueue            Pointer to the queue structure.
      mutex             Cache_lock to acquire after awake.
      thread            Thread to be added

  RETURN VALUE
    none

  NOTES.
    Queue is represented by a circular list of the thread structures
    The list is single-linked of the type (*next), accessed by a pointer
    to the last element.

    The function protects against stray signals by verifying that the
    current thread is unlinked from the queue when awaking. However,
    since several threads can wait for the same event, it might be
    necessary for the caller of the function to check again if the
    condition for awake is indeed matched.
*/

static void wait_on_queue(KEYCACHE_WQUEUE *wqueue, mysql_mutex_t *mutex,
                          st_keycache_thread_var *thread) {
  st_keycache_thread_var *last;

  /* Add to queue. */
  DBUG_ASSERT(!thread->next);
  DBUG_ASSERT(!thread->prev); /* Not required, but must be true anyway. */
  if (!(last = wqueue->last_thread))
    thread->next = thread;
  else {
    thread->next = last->next;
    last->next = thread;
  }
  wqueue->last_thread = thread;

  /*
    Wait until thread is removed from queue by the signalling thread.
    The loop protects against stray signals.
  */
  do {
    mysql_cond_wait(&thread->suspend, mutex);
  } while (thread->next);
}

/*
  Remove all threads from queue signaling them to proceed

  SYNOPSIS
    release_whole_queue()
      wqueue            pointer to the queue structure

  RETURN VALUE
    none

  NOTES.
    See notes for wait_on_queue().
    When removed from the queue each thread is signaled via condition
    variable thread->suspend.
*/

static void release_whole_queue(KEYCACHE_WQUEUE *wqueue) {
  st_keycache_thread_var *last;
  st_keycache_thread_var *next;
  st_keycache_thread_var *thread;

  /* Queue may be empty. */
  if (!(last = wqueue->last_thread)) return;

  next = last->next;
  do {
    thread = next;
    /* Signal the thread. */
    mysql_cond_signal(&thread->suspend);
    /* Take thread from queue. */
    next = thread->next;
    thread->next = nullptr;
  } while (thread != last);

  /* Now queue is definitely empty. */
  wqueue->last_thread = nullptr;
}

/*
  Unlink a block from the chain of dirty/clean blocks
*/

static inline void unlink_changed(BLOCK_LINK *block) {
  DBUG_ASSERT(block->prev_changed && *block->prev_changed == block);
  if (block->next_changed)
    block->next_changed->prev_changed = block->prev_changed;
  *block->prev_changed = block->next_changed;

#if !defined(DBUG_OFF)
  /*
    This makes it easier to see it's not in a chain during debugging.
    And some DBUG_ASSERT() rely on it.
  */
  block->next_changed = nullptr;
  block->prev_changed = nullptr;
#endif
}

/*
  Link a block into the chain of dirty/clean blocks
*/

static inline void link_changed(BLOCK_LINK *block, BLOCK_LINK **phead) {
  DBUG_ASSERT(!block->next_changed);
  DBUG_ASSERT(!block->prev_changed);
  block->prev_changed = phead;
  if ((block->next_changed = *phead))
    (*phead)->prev_changed = &block->next_changed;
  *phead = block;
}

/*
  Link a block in a chain of clean blocks of a file.

  SYNOPSIS
    link_to_file_list()
      keycache		Key cache handle
      block             Block to relink
      file              File to be linked to
      unlink            If to unlink first

  DESCRIPTION
    Unlink a block from whichever chain it is linked in, if it's
    asked for, and link it to the chain of clean blocks of the
    specified file.

  NOTE
    Please do never set/clear BLOCK_CHANGED outside of
    link_to_file_list() or link_to_changed_list().
    You would risk to damage correct counting of changed blocks
    and to find blocks in the wrong hash.

  RETURN
    void
*/

static void link_to_file_list(KEY_CACHE *keycache, BLOCK_LINK *block, int file,
                              bool unlink_block) {
  DBUG_ASSERT(block->status & BLOCK_IN_USE);
  DBUG_ASSERT(block->hash_link && block->hash_link->block == block);
  DBUG_ASSERT(block->hash_link->file == file);
  if (unlink_block) unlink_changed(block);
  link_changed(block, &keycache->file_blocks[FILE_HASH(file)]);
  if (block->status & BLOCK_CHANGED) {
    block->status &= ~BLOCK_CHANGED;
    keycache->blocks_changed--;
    keycache->global_blocks_changed--;
  }
}

/*
  Re-link a block from the clean chain to the dirty chain of a file.

  SYNOPSIS
    link_to_changed_list()
      keycache		key cache handle
      block             block to relink

  DESCRIPTION
    Unlink a block from the chain of clean blocks of a file
    and link it to the chain of dirty blocks of the same file.

  NOTE
    Please do never set/clear BLOCK_CHANGED outside of
    link_to_file_list() or link_to_changed_list().
    You would risk to damage correct counting of changed blocks
    and to find blocks in the wrong hash.

  RETURN
    void
*/

static void link_to_changed_list(KEY_CACHE *keycache, BLOCK_LINK *block) {
  DBUG_ASSERT(block->status & BLOCK_IN_USE);
  DBUG_ASSERT(!(block->status & BLOCK_CHANGED));
  DBUG_ASSERT(block->hash_link && block->hash_link->block == block);

  unlink_changed(block);
  link_changed(block,
               &keycache->changed_blocks[FILE_HASH(block->hash_link->file)]);
  block->status |= BLOCK_CHANGED;
  keycache->blocks_changed++;
  keycache->global_blocks_changed++;
}

/*
  Link a block to the LRU chain at the beginning or at the end of
  one of two parts.

  SYNOPSIS
    link_block()
      keycache            pointer to a key cache data structure
      block               pointer to the block to link to the LRU chain
      hot                 <-> to link the block into the hot subchain
      at_end              <-> to link the block at the end of the subchain

  RETURN VALUE
    none

  NOTES.
    The LRU ring is represented by a circular list of block structures.
    The list is double-linked of the type (**prev,*next) type.
    The LRU ring is divided into two parts - hot and warm.
    There are two pointers to access the last blocks of these two
    parts. The beginning of the warm part follows right after the
    end of the hot part.
    Only blocks of the warm part can be used for eviction.
    The first block from the beginning of this subchain is always
    taken for eviction (keycache->last_used->next)

    LRU chain:       +------+   H O T    +------+
                +----| end  |----...<----| beg  |----+
                |    +------+last        +------+    |
                v<-link in latest hot (new end)      |
                |     link in latest warm (new end)->^
                |    +------+  W A R M   +------+    |
                +----| beg  |---->...----| end  |----+
                     +------+            +------+ins
                  first for eviction

    It is also possible that the block is selected for eviction and thus
    not linked in the LRU ring.
*/

static void link_block(KEY_CACHE *keycache, BLOCK_LINK *block, bool hot,
                       bool at_end) {
  BLOCK_LINK *ins;
  BLOCK_LINK **pins;

  DBUG_ASSERT((block->status & ~BLOCK_CHANGED) == (BLOCK_READ | BLOCK_IN_USE));
  DBUG_ASSERT(block->hash_link); /*backptr to block NULL from free_block()*/
  DBUG_ASSERT(!block->requests);
  DBUG_ASSERT(block->prev_changed && *block->prev_changed == block);
  DBUG_ASSERT(!block->next_used);
  DBUG_ASSERT(!block->prev_used);

  if (!hot && keycache->waiting_for_block.last_thread) {
    /* Signal that in the LRU warm sub-chain an available block has appeared */
    st_keycache_thread_var *last_thread =
        keycache->waiting_for_block.last_thread;
    st_keycache_thread_var *first_thread = last_thread->next;
    st_keycache_thread_var *next_thread = first_thread;
    HASH_LINK *hash_link = (HASH_LINK *)first_thread->opt_info;
    st_keycache_thread_var *thread;
    do {
      thread = next_thread;
      next_thread = thread->next;
      /*
         We notify about the event all threads that ask
         for the same page as the first thread in the queue
      */
      if ((HASH_LINK *)thread->opt_info == hash_link) {
        mysql_cond_signal(&thread->suspend);
        unlink_from_queue(&keycache->waiting_for_block, thread);
        block->requests++;
      }
    } while (thread != last_thread);
    hash_link->block = block;
    /*
      NOTE: We assigned the block to the hash_link and signalled the
      requesting thread(s). But it is possible that other threads runs
      first. These threads see the hash_link assigned to a block which
      is assigned to another hash_link and not marked BLOCK_IN_SWITCH.
      This can be a problem for functions that do not select the block
      via its hash_link: flush and free. They do only see a block which
      is in a "normal" state and don't know that it will be evicted soon.

      We cannot set BLOCK_IN_SWITCH here because only one of the
      requesting threads must handle the eviction. All others must wait
      for it to complete. If we set the flag here, the threads would not
      know who is in charge of the eviction. Without the flag, the first
      thread takes the stick and sets the flag.

      But we need to note in the block that is has been selected for
      eviction. It must not be freed. The evicting thread will not
      expect the block in the free list. Before freeing we could also
      check if block->requests > 1. But I think including another flag
      in the check of block->status is slightly more efficient and
      probably easier to read.
    */
    block->status |= BLOCK_IN_EVICTION;
    return;
  }

  pins = hot ? &keycache->used_ins : &keycache->used_last;
  ins = *pins;
  if (ins) {
    ins->next_used->prev_used = &block->next_used;
    block->next_used = ins->next_used;
    block->prev_used = &ins->next_used;
    ins->next_used = block;
    if (at_end) *pins = block;
  } else {
    /* The LRU ring is empty. Let the block point to itself. */
    keycache->used_last = keycache->used_ins = block->next_used = block;
    block->prev_used = &block->next_used;
  }
  DBUG_ASSERT((ulong)keycache->blocks_available <= keycache->blocks_used);
}

/*
  Unlink a block from the LRU chain

  SYNOPSIS
    unlink_block()
      keycache            pointer to a key cache data structure
      block               pointer to the block to unlink from the LRU chain

  RETURN VALUE
    none

  NOTES.
    See NOTES for link_block
*/

static void unlink_block(KEY_CACHE *keycache, BLOCK_LINK *block) {
  DBUG_ASSERT((block->status & ~BLOCK_CHANGED) == (BLOCK_READ | BLOCK_IN_USE));
  DBUG_ASSERT(block->hash_link); /*backptr to block NULL from free_block()*/
  DBUG_ASSERT(!block->requests);
  DBUG_ASSERT(block->prev_changed && *block->prev_changed == block);
  DBUG_ASSERT(block->next_used && block->prev_used &&
              (block->next_used->prev_used == &block->next_used) &&
              (*block->prev_used == block));
  if (block->next_used == block) /* The list contains only one member */
    keycache->used_last = keycache->used_ins = nullptr;
  else {
    block->next_used->prev_used = block->prev_used;
    *block->prev_used = block->next_used;
    if (keycache->used_last == block)
      keycache->used_last = STRUCT_PTR(BLOCK_LINK, next_used, block->prev_used);
    if (keycache->used_ins == block)
      keycache->used_ins = STRUCT_PTR(BLOCK_LINK, next_used, block->prev_used);
  }
  block->next_used = nullptr;
#if !defined(DBUG_OFF)
  /*
    This makes it easier to see it's not in a chain during debugging.
    And some DBUG_ASSERT() rely on it.
  */
  block->prev_used = nullptr;
#endif
}

/*
  Register requests for a block.

  SYNOPSIS
    reg_requests()
      keycache          Pointer to a key cache data structure.
      block             Pointer to the block to register a request on.
      count             Number of requests. Always 1.

  NOTE
    The first request unlinks the block from the LRU ring. This means
    that it is protected against eveiction.

  RETURN
    void
*/
static void reg_requests(KEY_CACHE *keycache, BLOCK_LINK *block, int count) {
  DBUG_ASSERT(block->status & BLOCK_IN_USE);
  DBUG_ASSERT(block->hash_link);

  if (!block->requests) unlink_block(keycache, block);
  block->requests += count;
}

/*
  Unregister request for a block
  linking it to the LRU chain if it's the last request

  SYNOPSIS
    unreg_request()
    keycache            pointer to a key cache data structure
    block               pointer to the block to link to the LRU chain
    at_end              <-> to link the block at the end of the LRU chain

  RETURN VALUE
    none

  NOTES.
    Every linking to the LRU ring decrements by one a special block
    counter (if it's positive). If the at_end parameter is true the block is
    added either at the end of warm sub-chain or at the end of hot sub-chain.
    It is added to the hot subchain if its counter is zero and number of
    blocks in warm sub-chain is not less than some low limit (determined by
    the division_limit parameter). Otherwise the block is added to the warm
    sub-chain. If the at_end parameter is false the block is always added
    at beginning of the warm sub-chain.
    Thus a warm block can be promoted to the hot sub-chain when its counter
    becomes zero for the first time.
    At the same time  the block at the very beginning of the hot subchain
    might be moved to the beginning of the warm subchain if it stays untouched
    for a too long time (this time is determined by parameter age_threshold).

    It is also possible that the block is selected for eviction and thus
    not linked in the LRU ring.
*/

static void unreg_request(KEY_CACHE *keycache, BLOCK_LINK *block, int at_end) {
  DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
  DBUG_ASSERT(block->hash_link); /*backptr to block NULL from free_block()*/
  DBUG_ASSERT(block->requests);
  DBUG_ASSERT(block->prev_changed && *block->prev_changed == block);
  DBUG_ASSERT(!block->next_used);
  DBUG_ASSERT(!block->prev_used);
  /*
    Unregister the request, but do not link erroneous blocks into the
    LRU ring.
  */
  if (!--block->requests && !(block->status & BLOCK_ERROR)) {
    bool hot;
    if (block->hits_left) block->hits_left--;
    hot = !block->hits_left && at_end &&
          keycache->warm_blocks > keycache->min_warm_blocks;
    if (hot) {
      if (block->temperature == BLOCK_WARM) keycache->warm_blocks--;
      block->temperature = BLOCK_HOT;
    }
    link_block(keycache, block, hot, at_end);
    block->last_hit_time = keycache->keycache_time;
    keycache->keycache_time++;
    /*
      At this place, the block might be in the LRU ring or not. If an
      evicter was waiting for a block, it was selected for eviction and
      not linked in the LRU ring.
    */

    /*
      Check if we should link a hot block to the warm block sub-chain.
      It is possible that we select the same block as above. But it can
      also be another block. In any case a block from the LRU ring is
      selected. In other words it works even if the above block was
      selected for eviction and not linked in the LRU ring. Since this
      happens only if the LRU ring is empty, the block selected below
      would be NULL and the rest of the function skipped.
    */
    block = keycache->used_ins;
    if (block && keycache->keycache_time - block->last_hit_time >
                     keycache->age_threshold) {
      unlink_block(keycache, block);
      link_block(keycache, block, false, false);
      if (block->temperature != BLOCK_WARM) {
        keycache->warm_blocks++;
        block->temperature = BLOCK_WARM;
      }
    }
  }
}

/*
  Remove a reader of the page in block
*/

static void remove_reader(BLOCK_LINK *block) {
  DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
  DBUG_ASSERT(block->hash_link && block->hash_link->block == block);
  DBUG_ASSERT(block->prev_changed && *block->prev_changed == block);
  DBUG_ASSERT(!block->next_used);
  DBUG_ASSERT(!block->prev_used);
  DBUG_ASSERT(block->hash_link->requests);

  if (!--block->hash_link->requests && block->condvar)
    mysql_cond_signal(block->condvar);
}

/*
  Wait until the last reader of the page in block
  signals on its termination
*/

static void wait_for_readers(KEY_CACHE *keycache, BLOCK_LINK *block,
                             st_keycache_thread_var *thread) {
  DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
  DBUG_ASSERT(!(block->status & (BLOCK_IN_FLUSH | BLOCK_CHANGED)));
  DBUG_ASSERT(block->hash_link);
  DBUG_ASSERT(block->hash_link->block == block);
  /* Linked in file_blocks or changed_blocks hash. */
  DBUG_ASSERT(block->prev_changed && *block->prev_changed == block);
  /* Not linked in LRU ring. */
  DBUG_ASSERT(!block->next_used);
  DBUG_ASSERT(!block->prev_used);
  while (block->hash_link->requests) {
    /* There must be no other waiter. We have no queue here. */
    DBUG_ASSERT(!block->condvar);
    block->condvar = &thread->suspend;
    mysql_cond_wait(&thread->suspend, &keycache->cache_lock);
    block->condvar = nullptr;
  }
}

/*
  Add a hash link to a bucket in the hash_table
*/

static inline void link_hash(HASH_LINK **start, HASH_LINK *hash_link) {
  if (*start) (*start)->prev = &hash_link->next;
  hash_link->next = *start;
  hash_link->prev = start;
  *start = hash_link;
}

/*
  Remove a hash link from the hash table
*/

static void unlink_hash(KEY_CACHE *keycache, HASH_LINK *hash_link) {
  DBUG_ASSERT(hash_link->requests == 0);
  if ((*hash_link->prev = hash_link->next))
    hash_link->next->prev = hash_link->prev;
  hash_link->block = nullptr;

  if (keycache->waiting_for_hash_link.last_thread) {
    /* Signal that a free hash link has appeared */
    st_keycache_thread_var *last_thread =
        keycache->waiting_for_hash_link.last_thread;
    st_keycache_thread_var *first_thread = last_thread->next;
    st_keycache_thread_var *next_thread = first_thread;
    KEYCACHE_PAGE *first_page = (KEYCACHE_PAGE *)(first_thread->opt_info);
    st_keycache_thread_var *thread;

    hash_link->file = first_page->file;
    hash_link->diskpos = first_page->filepos;
    do {
      KEYCACHE_PAGE *page;
      thread = next_thread;
      page = (KEYCACHE_PAGE *)thread->opt_info;
      next_thread = thread->next;
      /*
         We notify about the event all threads that ask
         for the same page as the first thread in the queue
      */
      if (page->file == hash_link->file &&
          page->filepos == hash_link->diskpos) {
        mysql_cond_signal(&thread->suspend);
        unlink_from_queue(&keycache->waiting_for_hash_link, thread);
      }
    } while (thread != last_thread);
    link_hash(
        &keycache
             ->hash_root[KEYCACHE_HASH(hash_link->file, hash_link->diskpos)],
        hash_link);
    return;
  }
  hash_link->next = keycache->free_hash_list;
  keycache->free_hash_list = hash_link;
}

/*
  Get the hash link for a page
*/

static HASH_LINK *get_hash_link(KEY_CACHE *keycache, int file, my_off_t filepos,
                                st_keycache_thread_var *thread) {
  HASH_LINK *hash_link, **start;
#ifndef DBUG_OFF
  int cnt;
#endif

restart:
  /*
     Find the bucket in the hash table for the pair (file, filepos);
     start contains the head of the bucket list,
     hash_link points to the first member of the list
  */
  hash_link = *(start = &keycache->hash_root[KEYCACHE_HASH(file, filepos)]);
#ifndef DBUG_OFF
  cnt = 0;
#endif
  /* Look for an element for the pair (file, filepos) in the bucket chain */
  while (hash_link &&
         (hash_link->diskpos != filepos || hash_link->file != file)) {
    hash_link = hash_link->next;
#ifndef DBUG_OFF
    cnt++;
    DBUG_ASSERT(cnt <= keycache->hash_links_used);
#endif
  }
  if (!hash_link) {
    /* There is no hash link in the hash table for the pair (file, filepos) */
    if (keycache->free_hash_list) {
      hash_link = keycache->free_hash_list;
      keycache->free_hash_list = hash_link->next;
    } else if (keycache->hash_links_used < keycache->hash_links) {
      hash_link = &keycache->hash_link_root[keycache->hash_links_used++];
    } else {
      /* Wait for a free hash link */
      KEYCACHE_PAGE page;
      page.file = file;
      page.filepos = filepos;
      thread->opt_info = (void *)&page;
      link_into_queue(&keycache->waiting_for_hash_link, thread);
      mysql_cond_wait(&thread->suspend, &keycache->cache_lock);
      thread->opt_info = nullptr;
      goto restart;
    }
    hash_link->file = file;
    hash_link->diskpos = filepos;
    link_hash(start, hash_link);
  }
  /* Register the request for the page */
  hash_link->requests++;

  return hash_link;
}

/*
  Get a block for the file page requested by a keycache read/write operation;
  If the page is not in the cache return a free block, if there is none
  return the lru block after saving its buffer if the page is dirty.

  SYNOPSIS

    find_key_block()
      keycache            pointer to a key cache data structure
      thread              pointer to thread specific variables
      file                handler for the file to read page from
      filepos             position of the page in the file
      init_hits_left      how initialize the block counter for the page
      wrmode              <-> get for writing
      page_st        out  {PAGE_READ,PAGE_TO_BE_READ,PAGE_WAIT_TO_BE_READ}

  RETURN VALUE
    Pointer to the found block if successful, 0 - otherwise

  NOTES.
    For the page from file positioned at filepos the function checks whether
    the page is in the key cache specified by the first parameter.
    If this is the case it immediately returns the block.
    If not, the function first chooses  a block for this page. If there is
    no not used blocks in the key cache yet, the function takes the block
    at the very beginning of the warm sub-chain. It saves the page in that
    block if it's dirty before returning the pointer to it.
    The function returns in the page_st parameter the following values:
      PAGE_READ         - if page already in the block,
      PAGE_TO_BE_READ   - if it is to be read yet by the current thread
      WAIT_TO_BE_READ   - if it is to be read by another thread
    If an error occurs THE BLOCK_ERROR bit is set in the block status.
    It might happen that there are no blocks in LRU chain (in warm part) -
    all blocks  are unlinked for some read/write operations. Then the function
    waits until first of this operations links any block back.
*/

static BLOCK_LINK *find_key_block(KEY_CACHE *keycache,
                                  st_keycache_thread_var *thread, File file,
                                  my_off_t filepos, int init_hits_left,
                                  int wrmode, int *page_st) {
  HASH_LINK *hash_link;
  BLOCK_LINK *block;
  int error = 0;
  int page_status;

  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("fd: %d  pos: %lu  wrmode: %d", file, (ulong)filepos, wrmode));

restart:
  /*
    If the flush phase of a resize operation fails, the cache is left
    unusable. This will be detected only after "goto restart".
  */
  if (!keycache->can_be_used) return nullptr;

  /*
    Find the hash_link for the requested file block (file, filepos). We
    do always get a hash_link here. It has registered our request so
    that no other thread can use it for another file block until we
    release the request (which is done by remove_reader() usually). The
    hash_link can have a block assigned to it or not. If there is a
    block, it may be assigned to this hash_link or not. In cases where a
    block is evicted from the cache, it is taken from the LRU ring and
    referenced by the new hash_link. But the block can still be assigned
    to its old hash_link for some time if it needs to be flushed first,
    or if there are other threads still reading it.

    Summary:
      hash_link is always returned.
      hash_link->block can be:
      - NULL or
      - not assigned to this hash_link or
      - assigned to this hash_link. If assigned, the block can have
        - invalid data (when freshly assigned) or
        - valid data. Valid data can be
          - changed over the file contents (dirty) or
          - not changed (clean).
  */
  hash_link = get_hash_link(keycache, file, filepos, thread);
  DBUG_ASSERT((hash_link->file == file) && (hash_link->diskpos == filepos));

  page_status = -1;
  if ((block = hash_link->block) && block->hash_link == hash_link &&
      (block->status & BLOCK_READ)) {
    /* Assigned block with valid (changed or unchanged) contents. */
    page_status = PAGE_READ;
  }
  /*
    else (page_status == -1)
      - block == NULL or
      - block not assigned to this hash_link or
      - block assigned but not yet read from file (invalid data).
  */

  if (keycache->in_resize) {
    /* This is a request during a resize operation */

    if (!block) {
      /*
        The file block is not in the cache. We don't need it in the
        cache: we are going to read or write directly to file. Cancel
        the request. We can simply decrement hash_link->requests because
        we did not release cache_lock since increasing it. So no other
        thread can wait for our request to become released.
      */
      if (hash_link->requests == 1) {
        /*
          We are the only one to request this hash_link (this file/pos).
          Free the hash_link.
        */
        hash_link->requests--;
        unlink_hash(keycache, hash_link);
        return nullptr;
      }

      /*
        More requests on the hash_link. Someone tries to evict a block
        for this hash_link (could have started before resizing started).
        This means that the LRU ring is empty. Otherwise a block could
        be assigned immediately. Behave like a thread that wants to
        evict a block for this file/pos. Add to the queue of threads
        waiting for a block. Wait until there is one assigned.

        Refresh the request on the hash-link so that it cannot be reused
        for another file/pos.
      */
      thread->opt_info = (void *)hash_link;
      link_into_queue(&keycache->waiting_for_block, thread);
      do {
        mysql_cond_wait(&thread->suspend, &keycache->cache_lock);
      } while (thread->next);
      thread->opt_info = nullptr;
      /*
        A block should now be assigned to the hash_link. But it may
        still need to be evicted. Anyway, we should re-check the
        situation. page_status must be set correctly.
      */
      hash_link->requests--;
      goto restart;
    } /* end of if (!block) */

    /*
      There is a block for this file/pos in the cache. Register a
      request on it. This unlinks it from the LRU ring (if it is there)
      and hence protects it against eviction (if not already in
      eviction). We need this for returning the block to the caller, for
      calling remove_reader() (for debugging purposes), and for calling
      free_block(). The only case where we don't need the request is if
      the block is in eviction. In that case we have to unregister the
      request later.
    */
    reg_requests(keycache, block, 1);

    if (page_status != PAGE_READ) {
      /*
        - block not assigned to this hash_link or
        - block assigned but not yet read from file (invalid data).

        This must be a block in eviction. It will be read soon. We need
        to wait here until this happened. Otherwise the caller could
        access a wrong block or a block which is in read. While waiting
        we cannot lose hash_link nor block. We have registered a request
        on the hash_link. Everything can happen to the block but changes
        in the hash_link -> block relationship. In other words:
        everything can happen to the block but free or another completed
        eviction.

        Note that we bahave like a secondary requestor here. We just
        cannot return with PAGE_WAIT_TO_BE_READ. This would work for
        read requests and writes on dirty blocks that are not in flush
        only. Waiting here on COND_FOR_REQUESTED works in all
        situations.
      */
      DBUG_ASSERT(
          ((block->hash_link != hash_link) &&
           (block->status & (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH))) ||
          ((block->hash_link == hash_link) && !(block->status & BLOCK_READ)));
      wait_on_queue(&block->wqueue[COND_FOR_REQUESTED], &keycache->cache_lock,
                    thread);
      /*
        Here we can trust that the block has been assigned to this
        hash_link (block->hash_link == hash_link) and read into the
        buffer (BLOCK_READ). The worst things possible here are that the
        block is in free (BLOCK_REASSIGNED). But the block is still
        assigned to the hash_link. The freeing thread waits until we
        release our request on the hash_link. The block must not be
        again in eviction because we registered an request on it before
        starting to wait.
      */
      DBUG_ASSERT(block->hash_link == hash_link);
      DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
      DBUG_ASSERT(!(block->status & (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH)));
    }
    /*
      The block is in the cache. Assigned to the hash_link. Valid data.
      Note that in case of page_st == PAGE_READ, the block can be marked
      for eviction. In any case it can be marked for freeing.
    */

    if (!wrmode) {
      /* A reader can just read the block. */
      *page_st = PAGE_READ;
      DBUG_ASSERT((hash_link->file == file) &&
                  (hash_link->diskpos == filepos) &&
                  (block->hash_link == hash_link));
      return block;
    }

    /*
      This is a writer. No two writers for the same block can exist.
      This must be assured by locks outside of the key cache.
    */
    DBUG_ASSERT(!(block->status & BLOCK_FOR_UPDATE) || fail_block(block));

    while (block->status & BLOCK_IN_FLUSH) {
      /*
        Wait until the block is flushed to file. Do not release the
        request on the hash_link yet to prevent that the block is freed
        or reassigned while we wait. While we wait, several things can
        happen to the block, including another flush. But the block
        cannot be reassigned to another hash_link until we release our
        request on it. But it can be marked BLOCK_REASSIGNED from free
        or eviction, while they wait for us to release the hash_link.
      */
      wait_on_queue(&block->wqueue[COND_FOR_SAVED], &keycache->cache_lock,
                    thread);
      /*
        If the flush phase failed, the resize could have finished while
        we waited here.
      */
      if (!keycache->in_resize) {
        remove_reader(block);
        unreg_request(keycache, block, 1);
        goto restart;
      }
      DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
      DBUG_ASSERT(!(block->status & BLOCK_FOR_UPDATE) || fail_block(block));
      DBUG_ASSERT(block->hash_link == hash_link);
    }

    if (block->status & BLOCK_CHANGED) {
      /*
        We want to write a block with changed contents. If the cache
        block size is bigger than the callers block size (e.g. MyISAM),
        the caller may replace part of the block only. Changes of the
        other part of the block must be preserved. Since the block has
        not yet been selected for flush, we can still add our changes.
      */
      *page_st = PAGE_READ;
      DBUG_ASSERT((hash_link->file == file) &&
                  (hash_link->diskpos == filepos) &&
                  (block->hash_link == hash_link));
      return block;
    }

    /*
      This is a write request for a clean block. We do not want to have
      new dirty blocks in the cache while resizing. We will free the
      block and write directly to file. If the block is in eviction or
      in free, we just let it go.

      Unregister from the hash_link. This must be done before freeing
      the block. And it must be done if not freeing the block. Because
      we could have waited above, we need to call remove_reader(). Other
      threads could wait for us to release our request on the hash_link.
    */
    remove_reader(block);

    /* If the block is not in eviction and not in free, we can free it. */
    if (!(block->status &
          (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH | BLOCK_REASSIGNED))) {
      /*
        Free block as we are going to write directly to file.
        Although we have an exlusive lock for the updated key part,
        the control can be yielded by the current thread as we might
        have unfinished readers of other key parts in the block
        buffer. Still we are guaranteed not to have any readers
        of the key part we are writing into until the block is
        removed from the cache as we set the BLOCK_REASSIGNED
        flag (see the code below that handles reading requests).
      */
      free_block(keycache, thread, block);
    } else {
      /*
        The block will be evicted/freed soon. Don't touch it in any way.
        Unregister the request that we registered above.
      */
      unreg_request(keycache, block, 1);

      /*
        The block is still assigned to the hash_link (the file/pos that
        we are going to write to). Wait until the eviction/free is
        complete. Otherwise the direct write could complete before all
        readers are done with the block. So they could read outdated
        data.

        Since we released our request on the hash_link, it can be reused
        for another file/pos. Hence we cannot just check for
        block->hash_link == hash_link. As long as the resize is
        proceeding the block cannot be reassigned to the same file/pos
        again. So we can terminate the loop when the block is no longer
        assigned to this file/pos.
      */
      do {
        wait_on_queue(&block->wqueue[COND_FOR_SAVED], &keycache->cache_lock,
                      thread);
        /*
          If the flush phase failed, the resize could have finished
          while we waited here.
        */
        if (!keycache->in_resize) goto restart;
      } while (block->hash_link && (block->hash_link->file == file) &&
               (block->hash_link->diskpos == filepos));
    }
    return nullptr;
  }

  if (page_status == PAGE_READ &&
      (block->status &
       (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH | BLOCK_REASSIGNED))) {
    /*
      This is a request for a block to be removed from cache. The block
      is assigned to this hash_link and contains valid data, but is
      marked for eviction or to be freed. Possible reasons why it has
      not yet been evicted/freed can be a flush before reassignment
      (BLOCK_IN_SWITCH), readers of the block have not finished yet
      (BLOCK_REASSIGNED), or the evicting thread did not yet awake after
      the block has been selected for it (BLOCK_IN_EVICTION).
    */

    /*
       Only reading requests can proceed until the old dirty page is flushed,
       all others are to be suspended, then resubmitted
    */
    if (!wrmode && !(block->status & BLOCK_REASSIGNED)) {
      /*
        This is a read request and the block not yet reassigned. We can
        register our request and proceed. This unlinks the block from
        the LRU ring and protects it against eviction.
      */
      reg_requests(keycache, block, 1);
    } else {
      /*
        Either this is a write request for a block that is in eviction
        or in free. We must not use it any more. Instead we must evict
        another block. But we cannot do this before the eviction/free is
        done. Otherwise we would find the same hash_link + block again
        and again.

        Or this is a read request for a block in eviction/free that does
        not require a flush, but waits for readers to finish with the
        block. We do not read this block to let the eviction/free happen
        as soon as possible. Again we must wait so that we don't find
        the same hash_link + block again and again.
      */
      DBUG_ASSERT(hash_link->requests);
      hash_link->requests--;
      wait_on_queue(&block->wqueue[COND_FOR_SAVED], &keycache->cache_lock,
                    thread);
      /*
        The block is no longer assigned to this hash_link.
        Get another one.
      */
      goto restart;
    }
  } else {
    /*
      This is a request for a new block or for a block not to be removed.
      Either
      - block == NULL or
      - block not assigned to this hash_link or
      - block assigned but not yet read from file,
      or
      - block assigned with valid (changed or unchanged) data and
      - it will not be reassigned/freed.
    */
    if (!block) {
      /* No block is assigned to the hash_link yet. */
      if (keycache->blocks_unused) {
        if (keycache->free_block_list) {
          /* There is a block in the free list. */
          block = keycache->free_block_list;
          keycache->free_block_list = block->next_used;
          block->next_used = nullptr;
        } else {
          size_t block_mem_offset;
          /* There are some never used blocks, take first of them */
          DBUG_ASSERT(keycache->blocks_used < (ulong)keycache->disk_blocks);
          block = &keycache->block_root[keycache->blocks_used];
          block_mem_offset =
              ((size_t)keycache->blocks_used) * keycache->key_cache_block_size;
          block->buffer = keycache->block_mem + block_mem_offset;
          keycache->blocks_used++;
          DBUG_ASSERT(!block->next_used);
        }
        DBUG_ASSERT(!block->prev_used);
        DBUG_ASSERT(!block->next_changed);
        DBUG_ASSERT(!block->prev_changed);
        DBUG_ASSERT(!block->hash_link);
        DBUG_ASSERT(!block->status);
        DBUG_ASSERT(!block->requests);
        keycache->blocks_unused--;
        block->status = BLOCK_IN_USE;
        block->length = 0;
        block->offset = keycache->key_cache_block_size;
        block->requests = 1;
        block->temperature = BLOCK_COLD;
        block->hits_left = init_hits_left;
        block->last_hit_time = 0;
        block->hash_link = hash_link;
        hash_link->block = block;
        link_to_file_list(keycache, block, file, false);
        page_status = PAGE_TO_BE_READ;
      } else {
        /*
          There are no free blocks and no never used blocks, use a block
          from the LRU ring.
        */

        if (!keycache->used_last) {
          /*
            The LRU ring is empty. Wait until a new block is added to
            it. Several threads might wait here for the same hash_link,
            all of them must get the same block. While waiting for a
            block, after a block is selected for this hash_link, other
            threads can run first before this one awakes. During this
            time interval other threads find this hash_link pointing to
            the block, which is still assigned to another hash_link. In
            this case the block is not marked BLOCK_IN_SWITCH yet, but
            it is marked BLOCK_IN_EVICTION.
          */

          thread->opt_info = (void *)hash_link;
          link_into_queue(&keycache->waiting_for_block, thread);
          do {
            mysql_cond_wait(&thread->suspend, &keycache->cache_lock);
          } while (thread->next);
          thread->opt_info = nullptr;
          /* Assert that block has a request registered. */
          DBUG_ASSERT(hash_link->block->requests);
          /* Assert that block is not in LRU ring. */
          DBUG_ASSERT(!hash_link->block->next_used);
          DBUG_ASSERT(!hash_link->block->prev_used);
        }

        /*
          If we waited above, hash_link->block has been assigned by
          link_block(). Otherwise it is still NULL. In the latter case
          we need to grab a block from the LRU ring ourselves.
        */
        block = hash_link->block;
        if (!block) {
          /* Select the last block from the LRU ring. */
          block = keycache->used_last->next_used;
          block->hits_left = init_hits_left;
          block->last_hit_time = 0;
          hash_link->block = block;
          /*
            Register a request on the block. This unlinks it from the
            LRU ring and protects it against eviction.
          */
          DBUG_ASSERT(!block->requests);
          reg_requests(keycache, block, 1);
          /*
            We do not need to set block->status|= BLOCK_IN_EVICTION here
            because we will set block->status|= BLOCK_IN_SWITCH
            immediately without releasing the lock in between. This does
            also support debugging. When looking at the block, one can
            see if the block has been selected by link_block() after the
            LRU ring was empty, or if it was grabbed directly from the
            LRU ring in this branch.
          */
        }

        /*
          If we had to wait above, there is a small chance that another
          thread grabbed this block for the same file block already. But
          in most cases the first condition is true.
        */
        if (block->hash_link != hash_link &&
            !(block->status & BLOCK_IN_SWITCH)) {
          /* this is a primary request for a new page */
          block->status |= BLOCK_IN_SWITCH;

          if (block->status & BLOCK_CHANGED) {
            /* The block contains a dirty page - push it out of the cache */

            if (block->status & BLOCK_IN_FLUSH) {
              /*
                The block is marked for flush. If we do not wait here,
                it could happen that we write the block, reassign it to
                another file block, then, before the new owner can read
                the new file block, the flusher writes the cache block
                (which still has the old contents) to the new file block!
              */
              wait_on_queue(&block->wqueue[COND_FOR_SAVED],
                            &keycache->cache_lock, thread);
              /*
                The block is marked BLOCK_IN_SWITCH. It should be left
                alone except for reading. No free, no write.
              */
              DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
              DBUG_ASSERT(!(block->status & (BLOCK_REASSIGNED | BLOCK_CHANGED |
                                             BLOCK_FOR_UPDATE)));
            } else {
              block->status |= BLOCK_IN_FLUSH | BLOCK_IN_FLUSHWRITE;
              /*
                BLOCK_IN_EVICTION may be true or not. Other flags must
                have a fixed value.
              */
              DBUG_ASSERT((block->status & ~BLOCK_IN_EVICTION) ==
                          (BLOCK_READ | BLOCK_IN_SWITCH | BLOCK_IN_FLUSH |
                           BLOCK_IN_FLUSHWRITE | BLOCK_CHANGED | BLOCK_IN_USE));
              DBUG_ASSERT(block->hash_link);

              mysql_mutex_unlock(&keycache->cache_lock);
              /*
                The call is thread safe because only the current
                thread might change the block->hash_link value
              */
              error = (int)my_pwrite(block->hash_link->file,
                                     block->buffer + block->offset,
                                     block->length - block->offset,
                                     block->hash_link->diskpos + block->offset,
                                     MYF(MY_NABP | MY_WAIT_IF_FULL));
              mysql_mutex_lock(&keycache->cache_lock);

              /* Block status must not have changed. */
              DBUG_ASSERT((block->status & ~BLOCK_IN_EVICTION) ==
                              (BLOCK_READ | BLOCK_IN_SWITCH | BLOCK_IN_FLUSH |
                               BLOCK_IN_FLUSHWRITE | BLOCK_CHANGED |
                               BLOCK_IN_USE) ||
                          fail_block(block));
              keycache->global_cache_write++;
            }
          }

          block->status |= BLOCK_REASSIGNED;
          /*
            The block comes from the LRU ring. It must have a hash_link
            assigned.
          */
          DBUG_ASSERT(block->hash_link);
          if (block->hash_link) {
            /*
              All pending requests for this page must be resubmitted.
              This must be done before waiting for readers. They could
              wait for the flush to complete. And we must also do it
              after the wait. Flushers might try to free the block while
              we wait. They would wait until the reassignment is
              complete. Also the block status must reflect the correct
              situation: The block is not changed nor in flush any more.
              Note that we must not change the BLOCK_CHANGED flag
              outside of link_to_file_list() so that it is always in the
              correct queue and the *blocks_changed counters are
              correct.
            */
            block->status &= ~(BLOCK_IN_FLUSH | BLOCK_IN_FLUSHWRITE);
            link_to_file_list(keycache, block, block->hash_link->file, true);
            release_whole_queue(&block->wqueue[COND_FOR_SAVED]);
            /*
              The block is still assigned to its old hash_link.
              Wait until all pending read requests
              for this page are executed
              (we could have avoided this waiting, if we had read
              a page in the cache in a sweep, without yielding control)
            */
            wait_for_readers(keycache, block, thread);
            DBUG_ASSERT(block->hash_link && block->hash_link->block == block &&
                        block->prev_changed);
            /* The reader must not have been a writer. */
            DBUG_ASSERT(!(block->status & BLOCK_CHANGED));

            /* Wake flushers that might have found the block in between. */
            release_whole_queue(&block->wqueue[COND_FOR_SAVED]);

            /* Remove the hash link for the old file block from the hash. */
            unlink_hash(keycache, block->hash_link);

            /*
              For sanity checks link_to_file_list() asserts that block
              and hash_link refer to each other. Hence we need to assign
              the hash_link first, but then we would not know if it was
              linked before. Hence we would not know if to unlink it. So
              unlink it here and call link_to_file_list(..., false).
            */
            unlink_changed(block);
          }
          block->status = error ? BLOCK_ERROR : BLOCK_IN_USE;
          block->length = 0;
          block->offset = keycache->key_cache_block_size;
          block->hash_link = hash_link;
          link_to_file_list(keycache, block, file, false);
          page_status = PAGE_TO_BE_READ;

          DBUG_ASSERT(block->hash_link->block == block);
          DBUG_ASSERT(hash_link->block->hash_link == hash_link);
        } else {
          /*
            Either (block->hash_link == hash_link),
            or     (block->status & BLOCK_IN_SWITCH).

            This is for secondary requests for a new file block only.
            Either it is already assigned to the new hash_link meanwhile
            (if we had to wait due to empty LRU), or it is already in
            eviction by another thread. Since this block has been
            grabbed from the LRU ring and attached to this hash_link,
            another thread cannot grab the same block from the LRU ring
            anymore. If the block is in eviction already, it must become
            attached to the same hash_link and as such destined for the
            same file block.
          */
          page_status =
              (((block->hash_link == hash_link) && (block->status & BLOCK_READ))
                   ? PAGE_READ
                   : PAGE_WAIT_TO_BE_READ);
        }
      }
    } else {
      /*
        Block is not NULL. This hash_link points to a block.
        Either
        - block not assigned to this hash_link (yet) or
        - block assigned but not yet read from file,
        or
        - block assigned with valid (changed or unchanged) data and
        - it will not be reassigned/freed.

        The first condition means hash_link points to a block in
        eviction. This is not necessarily marked by BLOCK_IN_SWITCH yet.
        But then it is marked BLOCK_IN_EVICTION. See the NOTE in
        link_block(). In both cases it is destined for this hash_link
        and its file block address. When this hash_link got its block
        address, the block was removed from the LRU ring and cannot be
        selected for eviction (for another hash_link) again.

        Register a request on the block. This is another protection
        against eviction.
      */
      DBUG_ASSERT(
          ((block->hash_link != hash_link) &&
           (block->status & (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH))) ||
          ((block->hash_link == hash_link) && !(block->status & BLOCK_READ)) ||
          ((block->status & BLOCK_READ) &&
           !(block->status & (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH))));
      reg_requests(keycache, block, 1);
      page_status =
          (((block->hash_link == hash_link) && (block->status & BLOCK_READ))
               ? PAGE_READ
               : PAGE_WAIT_TO_BE_READ);
    }
  }

  DBUG_ASSERT(page_status != -1);
  /* Same assert basically, but be very sure. */
  DBUG_ASSERT(block);
  /* Assert that block has a request and is not in LRU ring. */
  DBUG_ASSERT(block->requests);
  DBUG_ASSERT(!block->next_used);
  DBUG_ASSERT(!block->prev_used);
  /* Assert that we return the correct block. */
  DBUG_ASSERT((page_status == PAGE_WAIT_TO_BE_READ) ||
              ((block->hash_link->file == file) &&
               (block->hash_link->diskpos == filepos)));
  *page_st = page_status;
  return block;
}

/*
  Read into a key cache block buffer from disk.

  SYNOPSIS

    read_block()
      keycache            pointer to a key cache data structure
      thread_var          pointer to thread specific variables
      block               block to which buffer the data is to be read
      read_length         size of data to be read
      min_length          at least so much data must be read
      primary             <-> the current thread will read the data

  RETURN VALUE
    None

  NOTES.
    The function either reads a page data from file to the block buffer,
    or waits until another thread reads it. What page to read is determined
    by a block parameter - reference to a hash link for this page.
    If an error occurs THE BLOCK_ERROR bit is set in the block status.
    We do not report error when the size of successfully read
    portion is less than read_length, but not less than min_length.
*/

static void read_block(KEY_CACHE *keycache, st_keycache_thread_var *thread_var,
                       BLOCK_LINK *block, uint read_length, uint min_length,
                       bool primary) {
  size_t got_length;

  /* On entry cache_lock is locked */

  if (primary) {
    /*
      This code is executed only by threads that submitted primary
      requests. Until block->status contains BLOCK_READ, all other
      request for the block become secondary requests. For a primary
      request the block must be properly initialized.
    */
    DBUG_ASSERT(((block->status & ~BLOCK_FOR_UPDATE) == BLOCK_IN_USE) ||
                fail_block(block));
    DBUG_ASSERT((block->length == 0) || fail_block(block));
    DBUG_ASSERT((block->offset == keycache->key_cache_block_size) ||
                fail_block(block));
    DBUG_ASSERT((block->requests > 0) || fail_block(block));

    keycache->global_cache_read++;
    /* Page is not in buffer yet, is to be read from disk */
    mysql_mutex_unlock(&keycache->cache_lock);
    /*
      Here other threads may step in and register as secondary readers.
      They will register in block->wqueue[COND_FOR_REQUESTED].
    */
    got_length = my_pread(block->hash_link->file, block->buffer, read_length,
                          block->hash_link->diskpos, MYF(0));
    mysql_mutex_lock(&keycache->cache_lock);
    /*
      The block can now have been marked for free (in case of
      FLUSH_RELEASE). Otherwise the state must be unchanged.
    */
    DBUG_ASSERT(((block->status & ~(BLOCK_REASSIGNED | BLOCK_FOR_UPDATE)) ==
                 BLOCK_IN_USE) ||
                fail_block(block));
    DBUG_ASSERT((block->length == 0) || fail_block(block));
    DBUG_ASSERT((block->offset == keycache->key_cache_block_size) ||
                fail_block(block));
    DBUG_ASSERT((block->requests > 0) || fail_block(block));

    if (got_length < min_length)
      block->status |= BLOCK_ERROR;
    else {
      block->status |= BLOCK_READ;
      block->length = (int)got_length;
      /*
        Do not set block->offset here. If this block is marked
        BLOCK_CHANGED later, we want to flush only the modified part. So
        only a writer may set block->offset down from
        keycache->key_cache_block_size.
      */
    }
    /* Signal that all pending requests for this page now can be processed */
    release_whole_queue(&block->wqueue[COND_FOR_REQUESTED]);
  } else {
    /*
      This code is executed only by threads that submitted secondary
      requests. At this point it could happen that the cache block is
      not yet assigned to the hash_link for the requested file block.
      But at awake from the wait this should be the case. Unfortunately
      we cannot assert this here because we do not know the hash_link
      for the requested file block nor the file and position. So we have
      to assert this in the caller.
    */
    wait_on_queue(&block->wqueue[COND_FOR_REQUESTED], &keycache->cache_lock,
                  thread_var);
  }
}

/*
  Read a block of data from a cached file into a buffer;

  SYNOPSIS

    key_cache_read()
      keycache            pointer to a key cache data structure
      thread_var          pointer to thread specific variables
      file                handler for the file for the block of data to be read
      filepos             position of the block of data in the file
      level               determines the weight of the data
      buff                buffer to where the data must be placed
      length              length of the buffer
      block_length        length of the block in the key cache buffer
      return_buffer       return pointer to the key cache buffer with the data

  RETURN VALUE
    Returns address from where the data is placed if sucessful, 0 - otherwise.

  NOTES.
    The function ensures that a block of data of size length from file
    positioned at filepos is in the buffers for some key cache blocks.
    Then the function either copies the data into the buffer buff, or,
    if return_buffer is true, it just returns the pointer to the key cache
    buffer with the data.
    Filepos must be a multiple of 'block_length', but it doesn't
    have to be a multiple of key_cache_block_size;
*/

uchar *key_cache_read(KEY_CACHE *keycache, st_keycache_thread_var *thread_var,
                      File file, my_off_t filepos, int level, uchar *buff,
                      uint length, uint block_length,
                      int return_buffer MY_ATTRIBUTE((unused))) {
  bool locked_and_incremented = false;
  int error = 0;
  uchar *start = buff;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("fd: %u  pos: %lu  length: %u", (uint)file,
                       (ulong)filepos, length));

  if (keycache->key_cache_inited) {
    /* Key cache is used */
    BLOCK_LINK *block;
    uint read_length;
    uint offset;
    int page_st;

    /*
      When the key cache is once initialized, we use the cache_lock to
      reliably distinguish the cases of normal operation, resizing, and
      disabled cache. We always increment and decrement
      'cnt_for_resize_op' so that a resizer can wait for pending I/O.
    */
    mysql_mutex_lock(&keycache->cache_lock);
    /*
      Cache resizing has two phases: Flushing and re-initializing. In
      the flush phase read requests are allowed to bypass the cache for
      blocks not in the cache. find_key_block() returns NULL in this
      case.

      After the flush phase new I/O requests must wait until the
      re-initialization is done. The re-initialization can be done only
      if no I/O request is in progress. The reason is that
      key_cache_block_size can change. With enabled cache, I/O is done
      in chunks of key_cache_block_size. Every chunk tries to use a
      cache block first. If the block size changes in the middle, a
      block could be missed and old data could be read.
    */
    while (keycache->in_resize && !keycache->resize_in_flush)
      wait_on_queue(&keycache->resize_queue, &keycache->cache_lock, thread_var);
    /* Register the I/O for the next resize. */
    inc_counter_for_resize_op(keycache);
    locked_and_incremented = true;
    /* Requested data may not always be aligned to cache blocks. */
    offset = (uint)(filepos % keycache->key_cache_block_size);
    /* Read data in key_cache_block_size increments */
    do {
      /* Cache could be disabled in a later iteration. */
      if (!keycache->can_be_used) {
        goto no_key_cache;
      }
      /* Start reading at the beginning of the cache block. */
      filepos -= offset;
      /* Do not read beyond the end of the cache block. */
      read_length = std::min(length, keycache->key_cache_block_size - offset);
      DBUG_ASSERT(read_length > 0);

      if (block_length > keycache->key_cache_block_size || offset)
        return_buffer = 0;

      /* Request the cache block that matches file/pos. */
      keycache->global_cache_r_requests++;

      block = find_key_block(keycache, thread_var, file, filepos, level, 0,
                             &page_st);
      if (!block) {
        /*
          This happens only for requests submitted during key cache
          resize. The block is not in the cache and shall not go in.
          Read directly from file.
        */
        keycache->global_cache_read++;
        mysql_mutex_unlock(&keycache->cache_lock);
        error = (my_pread(file, (uchar *)buff, read_length, filepos + offset,
                          MYF(MY_NABP)) != 0);
        mysql_mutex_lock(&keycache->cache_lock);
        goto next_block;
      }
      if (!(block->status & BLOCK_ERROR)) {
        if (page_st != PAGE_READ) {
          /* The requested page is to be read into the block buffer */
          read_block(keycache, thread_var, block,
                     keycache->key_cache_block_size, read_length + offset,
                     page_st == PAGE_TO_BE_READ);
          /*
            A secondary request must now have the block assigned to the
            requested file block. It does not hurt to check it for
            primary requests too.
          */
          DBUG_ASSERT(keycache->can_be_used);
          DBUG_ASSERT(block->hash_link->file == file);
          DBUG_ASSERT(block->hash_link->diskpos == filepos);
          DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
        } else if (block->length < read_length + offset) {
          /*
            Impossible if nothing goes wrong:
            this could only happen if we are using a file with
            small key blocks and are trying to read outside the file
          */
          set_my_errno(-1);
          block->status |= BLOCK_ERROR;
        }
      }

      /* block status may have added BLOCK_ERROR in the above 'if'. */
      if (!(block->status & BLOCK_ERROR)) {
        {
          DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
          mysql_mutex_unlock(&keycache->cache_lock);

          /* Copy data from the cache buffer */
          memcpy(buff, block->buffer + offset, (size_t)read_length);

          mysql_mutex_lock(&keycache->cache_lock);
          DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
        }
      }

      remove_reader(block);

      /* Error injection for coverage testing. */
      DBUG_EXECUTE_IF("key_cache_read_block_error",
                      block->status |= BLOCK_ERROR;);

      /* Do not link erroneous blocks into the LRU ring, but free them. */
      if (!(block->status & BLOCK_ERROR)) {
        /*
          Link the block into the LRU ring if it's the last submitted
          request for the block. This enables eviction for the block.
        */
        unreg_request(keycache, block, 1);
      } else {
        free_block(keycache, thread_var, block);
        error = 1;
        break;
      }

    next_block:
      buff += read_length;
      filepos += read_length + offset;
      offset = 0;

    } while ((length -= read_length));
    goto end;
  }

no_key_cache:
  /* Key cache is not used */

  keycache->global_cache_r_requests++;
  keycache->global_cache_read++;

  if (locked_and_incremented) mysql_mutex_unlock(&keycache->cache_lock);
  if (my_pread(file, (uchar *)buff, length, filepos, MYF(MY_NABP))) error = 1;
  if (locked_and_incremented) mysql_mutex_lock(&keycache->cache_lock);

end:
  if (locked_and_incremented) {
    dec_counter_for_resize_op(keycache);
    mysql_mutex_unlock(&keycache->cache_lock);
  }
  DBUG_PRINT("exit", ("error: %d", error));
  return error ? (uchar *)nullptr : start;
}

/*
  Insert a block of file data from a buffer into key cache

  SYNOPSIS
    key_cache_insert()
    keycache            pointer to a key cache data structure
    thread_var          pointer to thread specific variables
    file                handler for the file to insert data from
    filepos             position of the block of data in the file to insert
    level               determines the weight of the data
    buff                buffer to read data from
    length              length of the data in the buffer

  NOTES
    This is used by MyISAM to move all blocks from a index file to the key
    cache

  RETURN VALUE
    0 if a success, 1 - otherwise.
*/

int key_cache_insert(KEY_CACHE *keycache, st_keycache_thread_var *thread_var,
                     File file, my_off_t filepos, int level, uchar *buff,
                     uint length) {
  int error = 0;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("fd: %u  pos: %lu  length: %u", (uint)file,
                       (ulong)filepos, length));

  if (keycache->key_cache_inited) {
    /* Key cache is used */
    BLOCK_LINK *block;
    uint read_length;
    uint offset;
    int page_st;
    bool locked_and_incremented = false;

    /*
      When the keycache is once initialized, we use the cache_lock to
      reliably distinguish the cases of normal operation, resizing, and
      disabled cache. We always increment and decrement
      'cnt_for_resize_op' so that a resizer can wait for pending I/O.
    */
    mysql_mutex_lock(&keycache->cache_lock);
    /*
      We do not load index data into a disabled cache nor into an
      ongoing resize.
    */
    if (!keycache->can_be_used || keycache->in_resize) goto no_key_cache;
    /* Register the pseudo I/O for the next resize. */
    inc_counter_for_resize_op(keycache);
    locked_and_incremented = true;
    /* Loaded data may not always be aligned to cache blocks. */
    offset = (uint)(filepos % keycache->key_cache_block_size);
    /* Load data in key_cache_block_size increments. */
    do {
      /* Cache could be disabled or resizing in a later iteration. */
      if (!keycache->can_be_used || keycache->in_resize) goto no_key_cache;
      /* Start loading at the beginning of the cache block. */
      filepos -= offset;
      /* Do not load beyond the end of the cache block. */
      read_length = std::min(length, keycache->key_cache_block_size - offset);
      DBUG_ASSERT(read_length > 0);

      /* The block has been read by the caller already. */
      keycache->global_cache_read++;
      /* Request the cache block that matches file/pos. */
      keycache->global_cache_r_requests++;
      block = find_key_block(keycache, thread_var, file, filepos, level, 0,
                             &page_st);
      if (!block) {
        /*
          This happens only for requests submitted during key cache
          resize. The block is not in the cache and shall not go in.
          Stop loading index data.
        */
        goto no_key_cache;
      }
      if (!(block->status & BLOCK_ERROR)) {
        if ((page_st == PAGE_WAIT_TO_BE_READ) ||
            ((page_st == PAGE_TO_BE_READ) &&
             (offset || (read_length < keycache->key_cache_block_size)))) {
          /*
            Either

            this is a secondary request for a block to be read into the
            cache. The block is in eviction. It is not yet assigned to
            the requested file block (It does not point to the right
            hash_link). So we cannot call remove_reader() on the block.
            And we cannot access the hash_link directly here. We need to
            wait until the assignment is complete. read_block() executes
            the correct wait when called with primary == false.

            Or

            this is a primary request for a block to be read into the
            cache and the supplied data does not fill the whole block.

            This function is called on behalf of a LOAD INDEX INTO CACHE
            statement, which is a read-only task and allows other
            readers. It is possible that a parallel running reader tries
            to access this block. If it needs more data than has been
            supplied here, it would report an error. To be sure that we
            have all data in the block that is available in the file, we
            read the block ourselves.

            Though reading again what the caller did read already is an
            expensive operation, we need to do this for correctness.
          */
          read_block(keycache, thread_var, block,
                     keycache->key_cache_block_size, read_length + offset,
                     (page_st == PAGE_TO_BE_READ));
          /*
            A secondary request must now have the block assigned to the
            requested file block. It does not hurt to check it for
            primary requests too.
          */
          DBUG_ASSERT(keycache->can_be_used);
          DBUG_ASSERT(block->hash_link->file == file);
          DBUG_ASSERT(block->hash_link->diskpos == filepos);
          DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
        } else if (page_st == PAGE_TO_BE_READ) {
          /*
            This is a new block in the cache. If we come here, we have
            data for the whole block.
          */
          DBUG_ASSERT(block->hash_link->requests);
          DBUG_ASSERT(block->status & BLOCK_IN_USE);
          DBUG_ASSERT((page_st == PAGE_TO_BE_READ) ||
                      (block->status & BLOCK_READ));

          mysql_mutex_unlock(&keycache->cache_lock);
          /*
            Here other threads may step in and register as secondary readers.
            They will register in block->wqueue[COND_FOR_REQUESTED].
          */

          /* Copy data from buff */
          memcpy(block->buffer + offset, buff, (size_t)read_length);

          mysql_mutex_lock(&keycache->cache_lock);
          DBUG_ASSERT(block->status & BLOCK_IN_USE);
          DBUG_ASSERT((page_st == PAGE_TO_BE_READ) ||
                      (block->status & BLOCK_READ));
          /*
            After the data is in the buffer, we can declare the block
            valid. Now other threads do not need to register as
            secondary readers any more. They can immediately access the
            block.
          */
          block->status |= BLOCK_READ;
          block->length = read_length + offset;
          /*
            Do not set block->offset here. If this block is marked
            BLOCK_CHANGED later, we want to flush only the modified part. So
            only a writer may set block->offset down from
            keycache->key_cache_block_size.
          */
          /* Signal all pending requests. */
          release_whole_queue(&block->wqueue[COND_FOR_REQUESTED]);
        } else {
          /*
            page_st == PAGE_READ. The block is in the buffer. All data
            must already be present. Blocks are always read with all
            data available on file. Assert that the block does not have
            less contents than the preloader supplies. If the caller has
            data beyond block->length, it means that a file write has
            been done while this block was in cache and not extended
            with the new data. If the condition is met, we can simply
            ignore the block.
          */
          DBUG_ASSERT((page_st == PAGE_READ) &&
                      (read_length + offset <= block->length));
        }

        /*
          A secondary request must now have the block assigned to the
          requested file block. It does not hurt to check it for primary
          requests too.
        */
        DBUG_ASSERT(block->hash_link->file == file);
        DBUG_ASSERT(block->hash_link->diskpos == filepos);
        DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
      } /* end of if (!(block->status & BLOCK_ERROR)) */

      remove_reader(block);

      /* Error injection for coverage testing. */
      DBUG_EXECUTE_IF("key_cache_insert_block_error",
                      block->status |= BLOCK_ERROR;
                      errno = EIO;);

      /* Do not link erroneous blocks into the LRU ring, but free them. */
      if (!(block->status & BLOCK_ERROR)) {
        /*
          Link the block into the LRU ring if it's the last submitted
          request for the block. This enables eviction for the block.
        */
        unreg_request(keycache, block, 1);
      } else {
        free_block(keycache, thread_var, block);
        error = 1;
        break;
      }

      buff += read_length;
      filepos += read_length + offset;
      offset = 0;

    } while ((length -= read_length));

  no_key_cache:
    if (locked_and_incremented) dec_counter_for_resize_op(keycache);
    mysql_mutex_unlock(&keycache->cache_lock);
  }
  return error;
}

/*
  Write a buffer into a cached file.

  SYNOPSIS

    key_cache_write()
      keycache            pointer to a key cache data structure
      thread_var          pointer to thread specific variables
      file                handler for the file to write data to
      filepos             position in the file to write data to
      level               determines the weight of the data
      buff                buffer with the data
      length              length of the buffer
      dont_write          if is 0 then all dirty pages involved in writing
                          should have been flushed from key cache

  RETURN VALUE
    0 if a success, 1 - otherwise.

  NOTES.
    The function copies the data of size length from buff into buffers
    for key cache blocks that are  assigned to contain the portion of
    the file starting with position filepos.
    It ensures that this data is flushed to the file if dont_write is false.
    Filepos must be a multiple of 'block_length', but it doesn't
    have to be a multiple of key_cache_block_size;

    dont_write is always true in the server (info->lock_type is never F_UNLCK).
*/

int key_cache_write(KEY_CACHE *keycache, st_keycache_thread_var *thread_var,
                    File file, my_off_t filepos, int level, uchar *buff,
                    uint length, uint block_length MY_ATTRIBUTE((unused)),
                    int dont_write) {
  bool locked_and_incremented = false;
  int error = 0;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("fd: %u  pos: %lu  length: %u  block_length: %u"
                       "  key_block_length: %u",
                       (uint)file, (ulong)filepos, length, block_length,
                       keycache ? keycache->key_cache_block_size : 0));

  if (!dont_write) {
    /* purecov: begin inspected */
    /* Not used in the server. */
    /* Force writing from buff into disk. */
    keycache->global_cache_w_requests++;
    keycache->global_cache_write++;
    if (my_pwrite(file, buff, length, filepos, MYF(MY_NABP | MY_WAIT_IF_FULL)))
      return 1;
    /* purecov: end */
  }

  if (keycache->key_cache_inited) {
    /* Key cache is used */
    BLOCK_LINK *block;
    uint read_length;
    uint offset;
    int page_st;

    /*
      When the key cache is once initialized, we use the cache_lock to
      reliably distinguish the cases of normal operation, resizing, and
      disabled cache. We always increment and decrement
      'cnt_for_resize_op' so that a resizer can wait for pending I/O.
    */
    mysql_mutex_lock(&keycache->cache_lock);
    /*
      Cache resizing has two phases: Flushing and re-initializing. In
      the flush phase write requests can modify dirty blocks that are
      not yet in flush. Otherwise they are allowed to bypass the cache.
      find_key_block() returns NULL in both cases (clean blocks and
      non-cached blocks).

      After the flush phase new I/O requests must wait until the
      re-initialization is done. The re-initialization can be done only
      if no I/O request is in progress. The reason is that
      key_cache_block_size can change. With enabled cache I/O is done in
      chunks of key_cache_block_size. Every chunk tries to use a cache
      block first. If the block size changes in the middle, a block
      could be missed and data could be written below a cached block.
    */
    while (keycache->in_resize && !keycache->resize_in_flush)
      wait_on_queue(&keycache->resize_queue, &keycache->cache_lock, thread_var);
    /* Register the I/O for the next resize. */
    inc_counter_for_resize_op(keycache);
    locked_and_incremented = true;
    /* Requested data may not always be aligned to cache blocks. */
    offset = (uint)(filepos % keycache->key_cache_block_size);
    /* Write data in key_cache_block_size increments. */
    do {
      /* Cache could be disabled in a later iteration. */
      if (!keycache->can_be_used) goto no_key_cache;

      /* Start writing at the beginning of the cache block. */
      filepos -= offset;
      /* Do not write beyond the end of the cache block. */
      read_length = std::min(length, keycache->key_cache_block_size - offset);
      DBUG_ASSERT(read_length > 0);

      /* Request the cache block that matches file/pos. */
      keycache->global_cache_w_requests++;
      block = find_key_block(keycache, thread_var, file, filepos, level, 1,
                             &page_st);
      if (!block) {
        /*
          This happens only for requests submitted during key cache
          resize. The block is not in the cache and shall not go in.
          Write directly to file.
        */
        if (dont_write) {
          /* Used in the server. */
          keycache->global_cache_write++;
          mysql_mutex_unlock(&keycache->cache_lock);
          if (my_pwrite(file, (uchar *)buff, read_length, filepos + offset,
                        MYF(MY_NABP | MY_WAIT_IF_FULL)))
            error = 1;
          mysql_mutex_lock(&keycache->cache_lock);
        }
        goto next_block;
      }
      /*
        Prevent block from flushing and from being selected for to be
        freed. This must be set when we release the cache_lock.
        However, we must not set the status of the block before it is
        assigned to this file/pos.
      */
      if (page_st != PAGE_WAIT_TO_BE_READ) block->status |= BLOCK_FOR_UPDATE;
      /*
        We must read the file block first if it is not yet in the cache
        and we do not replace all of its contents.

        In cases where the cache block is big enough to contain (parts
        of) index blocks of different indexes, our request can be
        secondary (PAGE_WAIT_TO_BE_READ). In this case another thread is
        reading the file block. If the read completes after us, it
        overwrites our new contents with the old contents. So we have to
        wait for the other thread to complete the read of this block.
        read_block() takes care for the wait.
      */
      if (!(block->status & BLOCK_ERROR) &&
          ((page_st == PAGE_TO_BE_READ &&
            (offset || read_length < keycache->key_cache_block_size)) ||
           (page_st == PAGE_WAIT_TO_BE_READ))) {
        read_block(keycache, thread_var, block,
                   offset + read_length >= keycache->key_cache_block_size
                       ? offset
                       : keycache->key_cache_block_size,
                   offset, (page_st == PAGE_TO_BE_READ));
        DBUG_ASSERT(keycache->can_be_used);
        DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
        /*
          Prevent block from flushing and from being selected for to be
          freed. This must be set when we release the cache_lock.
          Here we set it in case we could not set it above.
        */
        block->status |= BLOCK_FOR_UPDATE;
      }
      /*
        The block should always be assigned to the requested file block
        here. It need not be BLOCK_READ when overwriting the whole block.
      */
      DBUG_ASSERT(block->hash_link->file == file);
      DBUG_ASSERT(block->hash_link->diskpos == filepos);
      DBUG_ASSERT(block->status & BLOCK_IN_USE);
      DBUG_ASSERT((page_st == PAGE_TO_BE_READ) || (block->status & BLOCK_READ));
      /*
        The block to be written must not be marked BLOCK_REASSIGNED.
        Otherwise it could be freed in dirty state or reused without
        another flush during eviction. It must also not be in flush.
        Otherwise the old contens may have been flushed already and
        the flusher could clear BLOCK_CHANGED without flushing the
        new changes again.
      */
      DBUG_ASSERT(!(block->status & BLOCK_REASSIGNED));

      while (block->status & BLOCK_IN_FLUSHWRITE) {
        /*
          Another thread is flushing the block. It was dirty already.
          Wait until the block is flushed to file. Otherwise we could
          modify the buffer contents just while it is written to file.
          An unpredictable file block contents would be the result.
          While we wait, several things can happen to the block,
          including another flush. But the block cannot be reassigned to
          another hash_link until we release our request on it.
        */
        wait_on_queue(&block->wqueue[COND_FOR_SAVED], &keycache->cache_lock,
                      thread_var);
        DBUG_ASSERT(keycache->can_be_used);
        DBUG_ASSERT(block->status & (BLOCK_READ | BLOCK_IN_USE));
        /* Still must not be marked for free. */
        DBUG_ASSERT(!(block->status & BLOCK_REASSIGNED));
        DBUG_ASSERT(block->hash_link && (block->hash_link->block == block));
      }

      /*
        We could perhaps release the cache_lock during access of the
        data like in the other functions. Locks outside of the key cache
        assure that readers and a writer do not access the same range of
        data. Parallel accesses should happen only if the cache block
        contains multiple index block(fragment)s. So different parts of
        the buffer would be read/written. An attempt to flush during
        memcpy() is prevented with BLOCK_FOR_UPDATE.
      */
      if (!(block->status & BLOCK_ERROR)) {
        mysql_mutex_unlock(&keycache->cache_lock);
        memcpy(block->buffer + offset, buff, (size_t)read_length);

        mysql_mutex_lock(&keycache->cache_lock);
      }

      if (!dont_write) {
        /* Not used in the server. buff has been written to disk at start. */
        if ((block->status & BLOCK_CHANGED) &&
            (!offset && read_length >= keycache->key_cache_block_size))
          link_to_file_list(keycache, block, block->hash_link->file, true);
      } else if (!(block->status & BLOCK_CHANGED))
        link_to_changed_list(keycache, block);
      block->status |= BLOCK_READ;
      /*
        Allow block to be selected for to be freed. Since it is marked
        BLOCK_CHANGED too, it won't be selected for to be freed without
        a flush.
      */
      block->status &= ~BLOCK_FOR_UPDATE;
      block->offset = std::min(block->offset, offset);
      block->length = std::max(block->length, read_length + offset);

      /* Threads may be waiting for the changes to be complete. */
      release_whole_queue(&block->wqueue[COND_FOR_REQUESTED]);

      /*
        If only a part of the cache block is to be replaced, and the
        rest has been read from file, then the cache lock has been
        released for I/O and it could be possible that another thread
        wants to evict or free the block and waits for it to be
        released. So we must not just decrement hash_link->requests, but
        also wake a waiting thread.
      */
      remove_reader(block);

      /* Error injection for coverage testing. */
      DBUG_EXECUTE_IF("key_cache_write_block_error",
                      block->status |= BLOCK_ERROR;);

      /* Do not link erroneous blocks into the LRU ring, but free them. */
      if (!(block->status & BLOCK_ERROR)) {
        /*
          Link the block into the LRU ring if it's the last submitted
          request for the block. This enables eviction for the block.
        */
        unreg_request(keycache, block, 1);
      } else {
        /* Pretend a "clean" block to avoid complications. */
        block->status &= ~(BLOCK_CHANGED);
        free_block(keycache, thread_var, block);
        error = 1;
        break;
      }

    next_block:
      buff += read_length;
      filepos += read_length + offset;
      offset = 0;

    } while ((length -= read_length));
    goto end;
  }

no_key_cache:
  /* Key cache is not used */
  if (dont_write) {
    /* Used in the server. */
    keycache->global_cache_w_requests++;
    keycache->global_cache_write++;
    if (locked_and_incremented) mysql_mutex_unlock(&keycache->cache_lock);
    if (my_pwrite(file, (uchar *)buff, length, filepos,
                  MYF(MY_NABP | MY_WAIT_IF_FULL)))
      error = 1;
    if (locked_and_incremented) mysql_mutex_lock(&keycache->cache_lock);
  }

end:
  if (locked_and_incremented) {
    dec_counter_for_resize_op(keycache);
    mysql_mutex_unlock(&keycache->cache_lock);
  }

  return error;
}

/*
  Free block.

  SYNOPSIS
    free_block()
      keycache          Pointer to a key cache data structure
      thread_var        Pointer to thread specific variables
      block             Pointer to the block to free

  DESCRIPTION
    Remove reference to block from hash table.
    Remove block from the chain of clean blocks.
    Add block to the free list.

  NOTE
    Block must not be free (status == 0).
    Block must not be in free_block_list.
    Block must not be in the LRU ring.
    Block must not be in eviction (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH).
    Block must not be in free (BLOCK_REASSIGNED).
    Block must not be in flush (BLOCK_IN_FLUSH).
    Block must not be dirty (BLOCK_CHANGED).
    Block must not be in changed_blocks (dirty) hash.
    Block must be in file_blocks (clean) hash.
    Block must refer to a hash_link.
    Block must have a request registered on it.
*/

static void free_block(KEY_CACHE *keycache, st_keycache_thread_var *thread_var,
                       BLOCK_LINK *block) {
  /*
    Assert that the block is not free already. And that it is in a clean
    state. Note that the block might just be assigned to a hash_link and
    not yet read (BLOCK_READ may not be set here). In this case a reader
    is registered in the hash_link and free_block() will wait for it
    below.
  */
  DBUG_ASSERT((block->status & BLOCK_IN_USE) &&
              !(block->status &
                (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH | BLOCK_REASSIGNED |
                 BLOCK_IN_FLUSH | BLOCK_CHANGED | BLOCK_FOR_UPDATE)));
  /* Assert that the block is in a file_blocks chain. */
  DBUG_ASSERT(block->prev_changed && *block->prev_changed == block);
  /* Assert that the block is not in the LRU ring. */
  DBUG_ASSERT(!block->next_used && !block->prev_used);
  /*
    IMHO the below condition (if()) makes no sense. I can't see how it
    could be possible that free_block() is entered with a NULL hash_link
    pointer. The only place where it can become NULL is in free_block()
    (or before its first use ever, but for those blocks free_block() is
    not called). I don't remove the conditional as it cannot harm, but
    place an DBUG_ASSERT to confirm my hypothesis. Eventually the
    condition (if()) can be removed.
  */
  DBUG_ASSERT(block->hash_link && block->hash_link->block == block);
  if (block->hash_link) {
    /*
      While waiting for readers to finish, new readers might request the
      block. But since we set block->status|= BLOCK_REASSIGNED, they
      will wait on block->wqueue[COND_FOR_SAVED]. They must be signalled
      later.
    */
    block->status |= BLOCK_REASSIGNED;
    wait_for_readers(keycache, block, thread_var);
    /*
      The block must not have been freed by another thread. Repeat some
      checks. An additional requirement is that it must be read now
      (BLOCK_READ).
    */
    DBUG_ASSERT(block->hash_link && block->hash_link->block == block);
    DBUG_ASSERT(
        (block->status & (BLOCK_READ | BLOCK_IN_USE | BLOCK_REASSIGNED)) &&
        !(block->status & (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH |
                           BLOCK_IN_FLUSH | BLOCK_CHANGED | BLOCK_FOR_UPDATE)));
    DBUG_ASSERT(block->prev_changed && *block->prev_changed == block);
    DBUG_ASSERT(!block->prev_used);
    /*
      Unset BLOCK_REASSIGNED again. If we hand the block to an evicting
      thread (through unreg_request() below), other threads must not see
      this flag. They could become confused.
    */
    block->status &= ~BLOCK_REASSIGNED;
    /*
      Do not release the hash_link until the block is off all lists.
      At least not if we hand it over for eviction in unreg_request().
    */
  }

  /*
    Unregister the block request and link the block into the LRU ring.
    This enables eviction for the block. If the LRU ring was empty and
    threads are waiting for a block, then the block wil be handed over
    for eviction immediately. Otherwise we will unlink it from the LRU
    ring again, without releasing the lock in between. So decrementing
    the request counter and updating statistics are the only relevant
    operation in this case. Assert that there are no other requests
    registered.
  */
  DBUG_ASSERT(block->requests == 1);
  unreg_request(keycache, block, 0);
  /*
    Note that even without releasing the cache lock it is possible that
    the block is immediately selected for eviction by link_block() and
    thus not added to the LRU ring. In this case we must not touch the
    block any more.
  */
  if (block->status & BLOCK_IN_EVICTION) return;

  /* Error blocks are not put into the LRU ring. */
  if (!(block->status & BLOCK_ERROR)) {
    /* Here the block must be in the LRU ring. Unlink it again. */
    DBUG_ASSERT(block->next_used && block->prev_used &&
                *block->prev_used == block);
    unlink_block(keycache, block);
  }
  if (block->temperature == BLOCK_WARM) keycache->warm_blocks--;
  block->temperature = BLOCK_COLD;

  /* Remove from file_blocks hash. */
  unlink_changed(block);

  /* Remove reference to block from hash table. */
  unlink_hash(keycache, block->hash_link);
  block->hash_link = nullptr;

  block->status = 0;
  block->length = 0;
  block->offset = keycache->key_cache_block_size;

  /* Enforced by unlink_changed(), but just to be sure. */
  DBUG_ASSERT(!block->next_changed && !block->prev_changed);
  /* Enforced by unlink_block(): not in LRU ring nor in free_block_list. */
  DBUG_ASSERT(!block->next_used && !block->prev_used);
  /* Insert the free block in the free list. */
  block->next_used = keycache->free_block_list;
  keycache->free_block_list = block;
  /* Keep track of the number of currently unused blocks. */
  keycache->blocks_unused++;

  /* All pending requests for this page must be resubmitted. */
  release_whole_queue(&block->wqueue[COND_FOR_SAVED]);
}

/*
  Flush a portion of changed blocks to disk,
  free used blocks if requested
*/

static int flush_cached_blocks(KEY_CACHE *keycache,
                               st_keycache_thread_var *thread_var, File file,
                               BLOCK_LINK **cache, BLOCK_LINK **end,
                               enum flush_type type) {
  int error;
  int last_errno = 0;
  uint count = (uint)(end - cache);

  /* Don't lock the cache during the flush */
  mysql_mutex_unlock(&keycache->cache_lock);
  /*
     As all blocks referred in 'cache' are marked by BLOCK_IN_FLUSH
     we are guarunteed no thread will change them
  */
  std::sort(cache, cache + count, [](const BLOCK_LINK *a, const BLOCK_LINK *b) {
    return a->hash_link->diskpos < b->hash_link->diskpos;
  });

  mysql_mutex_lock(&keycache->cache_lock);
  /*
    Note: Do not break the loop. We have registered a request on every
    block in 'cache'. These must be unregistered by free_block() or
    unreg_request().
  */
  for (; cache != end; cache++) {
    BLOCK_LINK *block = *cache;

    /*
      If the block contents is going to be changed, we abandon the flush
      for this block. flush_key_blocks_int() will restart its search and
      handle the block properly.
    */
    if (!(block->status & BLOCK_FOR_UPDATE)) {
      /* Blocks coming here must have a certain status. */
      DBUG_ASSERT(block->hash_link);
      DBUG_ASSERT(block->hash_link->block == block);
      DBUG_ASSERT(block->hash_link->file == file);
      DBUG_ASSERT((block->status & ~BLOCK_IN_EVICTION) ==
                  (BLOCK_READ | BLOCK_IN_FLUSH | BLOCK_CHANGED | BLOCK_IN_USE));
      block->status |= BLOCK_IN_FLUSHWRITE;
      mysql_mutex_unlock(&keycache->cache_lock);
      error = (int)my_pwrite(file, block->buffer + block->offset,
                             block->length - block->offset,
                             block->hash_link->diskpos + block->offset,
                             MYF(MY_NABP | MY_WAIT_IF_FULL));
      mysql_mutex_lock(&keycache->cache_lock);
      keycache->global_cache_write++;
      if (error) {
        block->status |= BLOCK_ERROR;
        if (!last_errno) last_errno = errno ? errno : -1;
      }
      block->status &= ~BLOCK_IN_FLUSHWRITE;
      /* Block must not have changed status except BLOCK_FOR_UPDATE. */
      DBUG_ASSERT(block->hash_link);
      DBUG_ASSERT(block->hash_link->block == block);
      DBUG_ASSERT(block->hash_link->file == file);
      DBUG_ASSERT((block->status & ~(BLOCK_FOR_UPDATE | BLOCK_IN_EVICTION)) ==
                  (BLOCK_READ | BLOCK_IN_FLUSH | BLOCK_CHANGED | BLOCK_IN_USE));
      /*
        Set correct status and link in right queue for free or later use.
        free_block() must not see BLOCK_CHANGED and it may need to wait
        for readers of the block. These should not see the block in the
        wrong hash. If not freeing the block, we need to have it in the
        right queue anyway.
      */
      link_to_file_list(keycache, block, file, true);
    }
    block->status &= ~BLOCK_IN_FLUSH;
    /*
      Let to proceed for possible waiting requests to write to the block page.
      It might happen only during an operation to resize the key cache.
    */
    release_whole_queue(&block->wqueue[COND_FOR_SAVED]);
    /* type will never be FLUSH_IGNORE_CHANGED here */
    if (!(type == FLUSH_KEEP || type == FLUSH_FORCE_WRITE) &&
        !(block->status &
          (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH | BLOCK_FOR_UPDATE))) {
      /*
        Note that a request has been registered against the block in
        flush_key_blocks_int().
      */
      free_block(keycache, thread_var, block);
    } else {
      /*
        Link the block into the LRU ring if it's the last submitted
        request for the block. This enables eviction for the block.
        Note that a request has been registered against the block in
        flush_key_blocks_int().
      */
      unreg_request(keycache, block, 1);
    }

  } /* end of for ( ; cache != end ; cache++) */
  return last_errno;
}

/*
  Flush all key blocks for a file to disk, but don't do any mutex locks.

  SYNOPSIS
    flush_key_blocks_int()
      keycache            pointer to a key cache data structure
      thread_var          pointer to thread specific variables
      file                handler for the file to flush to
      flush_type          type of the flush

  NOTES
    This function doesn't do any mutex locks because it needs to be called both
    from flush_key_blocks and flush_all_key_blocks (the later one does the
    mutex lock in the resize_key_cache() function).

    We do only care about changed blocks that exist when the function is
    entered. We do not guarantee that all changed blocks of the file are
    flushed if more blocks change while this function is running.

  RETURN
    0   ok
    1  error
*/

static int flush_key_blocks_int(KEY_CACHE *keycache,
                                st_keycache_thread_var *thread_var, File file,
                                enum flush_type type) {
  BLOCK_LINK *cache_buff[FLUSH_CACHE], **cache;
  int last_errno = 0;
  int last_errcnt = 0;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("file: %d  blocks_used: %lu  blocks_changed: %lu", file,
                       keycache->blocks_used, keycache->blocks_changed));

  cache = cache_buff;
  if (keycache->disk_blocks > 0) {
    /* Key cache exists and flush is not disabled */
    int error = 0;
    uint count = FLUSH_CACHE;
    BLOCK_LINK **pos, **end;
    BLOCK_LINK *first_in_switch = nullptr;
    BLOCK_LINK *last_in_flush;
    BLOCK_LINK *last_link_for_update;
    BLOCK_LINK *block, *next;
#ifndef DBUG_OFF
    uint cnt = 0;
#endif

    if (type != FLUSH_IGNORE_CHANGED) {
      /*
         Count how many key blocks we have to cache to be able
         to flush all dirty pages with minimum seek moves
      */
      count = 0;
      for (block = keycache->changed_blocks[FILE_HASH(file)]; block;
           block = block->next_changed) {
        if ((block->hash_link->file == file) &&
            !(block->status & BLOCK_IN_FLUSH)) {
          count++;
          DBUG_ASSERT(count <= keycache->blocks_used);
        }
      }
      /*
        Allocate a new buffer only if its bigger than the one we have.
        Assure that we always have some entries for the case that new
        changed blocks appear while we need to wait for something.
      */
      if ((count > FLUSH_CACHE) &&
          !(cache = (BLOCK_LINK **)my_malloc(
                key_memory_KEY_CACHE, sizeof(BLOCK_LINK *) * count, MYF(0))))
        cache = cache_buff;
      /*
        After a restart there could be more changed blocks than now.
        So we should not let count become smaller than the fixed buffer.
      */
      if (cache == cache_buff) count = FLUSH_CACHE;
    }

    /* Retrieve the blocks and write them to a buffer to be flushed */
  restart:
    last_in_flush = nullptr;
    last_link_for_update = nullptr;
    end = (pos = cache) + count;
    for (block = keycache->changed_blocks[FILE_HASH(file)]; block;
         block = next) {
#ifndef DBUG_OFF
      cnt++;
      DBUG_ASSERT(cnt <= keycache->blocks_used);
#endif
      next = block->next_changed;
      if (block->hash_link->file == file) {
        if (!(block->status & (BLOCK_IN_FLUSH | BLOCK_FOR_UPDATE))) {
          /*
            Note: The special handling of BLOCK_IN_SWITCH is obsolete
            since we set BLOCK_IN_FLUSH if the eviction includes a
            flush. It can be removed in a later version.
          */
          if (!(block->status & BLOCK_IN_SWITCH)) {
            /*
              We care only for the blocks for which flushing was not
              initiated by another thread and which are not in eviction.
              Registering a request on the block unlinks it from the LRU
              ring and protects against eviction.
            */
            reg_requests(keycache, block, 1);
            if (type != FLUSH_IGNORE_CHANGED) {
              /* It's not a temporary file */
              if (pos == end) {
                /*
                  This should happen relatively seldom. Remove the
                  request because we won't do anything with the block
                  but restart and pick it again in the next iteration.
                */
                unreg_request(keycache, block, 0);
                /*
                  This happens only if there is not enough
                  memory for the big block
                */
                if ((error = flush_cached_blocks(keycache, thread_var, file,
                                                 cache, end, type))) {
                  /* Do not loop infinitely trying to flush in vain. */
                  if ((last_errno == error) && (++last_errcnt > 5)) goto err;
                  last_errno = error;
                }
                /*
                  Restart the scan as some other thread might have changed
                  the changed blocks chain: the blocks that were in switch
                  state before the flush started have to be excluded
                */
                goto restart;
              }
              /*
                Mark the block with BLOCK_IN_FLUSH in order not to let
                other threads to use it for new pages and interfere with
                our sequence of flushing dirty file pages. We must not
                set this flag before actually putting the block on the
                write burst array called 'cache'.
              */
              block->status |= BLOCK_IN_FLUSH;
              /* Add block to the array for a write burst. */
              *pos++ = block;
            } else {
              /* It's a temporary file */
              DBUG_ASSERT(!(block->status & BLOCK_REASSIGNED));
              /*
                free_block() must not be called with BLOCK_CHANGED. Note
                that we must not change the BLOCK_CHANGED flag outside of
                link_to_file_list() so that it is always in the correct
                queue and the *blocks_changed counters are correct.
              */
              link_to_file_list(keycache, block, file, true);
              if (!(block->status & (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH))) {
                /* A request has been registered against the block above. */
                free_block(keycache, thread_var, block);
              } else {
                /*
                  Link the block into the LRU ring if it's the last
                  submitted request for the block. This enables eviction
                  for the block. A request has been registered against
                  the block above.
                */
                unreg_request(keycache, block, 1);
              }
            }
          } else {
            /*
              Link the block into a list of blocks 'in switch'.

              WARNING: Here we introduce a place where a changed block
              is not in the changed_blocks hash! This is acceptable for
              a BLOCK_IN_SWITCH. Never try this for another situation.
              Other parts of the key cache code rely on changed blocks
              being in the changed_blocks hash.
            */
            unlink_changed(block);
            link_changed(block, &first_in_switch);
          }
        } else if (type != FLUSH_KEEP) {
          /*
            During the normal flush at end of statement (FLUSH_KEEP) we
            do not need to ensure that blocks in flush or update by
            other threads are flushed. They will be flushed by them
            later. In all other cases we must assure that we do not have
            any changed block of this file in the cache when this
            function returns.
          */
          if (block->status & BLOCK_IN_FLUSH) {
            /* Remember the last block found to be in flush. */
            last_in_flush = block;
          } else {
            /* Remember the last block found to be selected for update. */
            last_link_for_update = block;
          }
        }
      }
    }
    if (pos != cache) {
      if ((error = flush_cached_blocks(keycache, thread_var, file, cache, pos,
                                       type))) {
        /* Do not loop inifnitely trying to flush in vain. */
        if ((last_errno == error) && (++last_errcnt > 5)) goto err;
        last_errno = error;
      }
      /*
        Do not restart here during the normal flush at end of statement
        (FLUSH_KEEP). We have now flushed at least all blocks that were
        changed when entering this function. In all other cases we must
        assure that we do not have any changed block of this file in the
        cache when this function returns.
      */
      if (type != FLUSH_KEEP) goto restart;
    }
    if (last_in_flush) {
      /*
        There are no blocks to be flushed by this thread, but blocks in
        flush by other threads. Wait until one of the blocks is flushed.
        Re-check the condition for last_in_flush. We may have unlocked
        the cache_lock in flush_cached_blocks(). The state of the block
        could have changed.
      */
      if (last_in_flush->status & BLOCK_IN_FLUSH)
        wait_on_queue(&last_in_flush->wqueue[COND_FOR_SAVED],
                      &keycache->cache_lock, thread_var);
      /* Be sure not to lose a block. They may be flushed in random order. */
      goto restart;
    }
    if (last_link_for_update) {
      /*
        There are no blocks to be flushed by this thread, but blocks for
        update by other threads. Wait until one of the blocks is updated.
        Re-check the condition for last_link_for_update. We may have unlocked
        the cache_lock in flush_cached_blocks(). The state of the block
        could have changed.
      */
      if (last_link_for_update->status & BLOCK_FOR_UPDATE)
        wait_on_queue(&last_link_for_update->wqueue[COND_FOR_REQUESTED],
                      &keycache->cache_lock, thread_var);
      /* The block is now changed. Flush it. */
      goto restart;
    }

    /*
      Wait until the list of blocks in switch is empty. The threads that
      are switching these blocks will relink them to clean file chains
      while we wait and thus empty the 'first_in_switch' chain.
    */
    while (first_in_switch) {
#ifndef DBUG_OFF
      cnt = 0;
#endif
      wait_on_queue(&first_in_switch->wqueue[COND_FOR_SAVED],
                    &keycache->cache_lock, thread_var);
#ifndef DBUG_OFF
      cnt++;
      DBUG_ASSERT(cnt <= keycache->blocks_used);
#endif
      /*
        Do not restart here. We have flushed all blocks that were
        changed when entering this function and were not marked for
        eviction. Other threads have now flushed all remaining blocks in
        the course of their eviction.
      */
    }

    if (!(type == FLUSH_KEEP || type == FLUSH_FORCE_WRITE)) {
      BLOCK_LINK *last_for_update = nullptr;
      BLOCK_LINK *last_in_switch = nullptr;
      uint total_found = 0;
      uint found;

      /*
        Finally free all clean blocks for this file.
        During resize this may be run by two threads in parallel.
      */
      do {
        found = 0;
        for (block = keycache->file_blocks[FILE_HASH(file)]; block;
             block = next) {
          /* Remember the next block. After freeing we cannot get at it. */
          next = block->next_changed;

          /* Changed blocks cannot appear in the file_blocks hash. */
          DBUG_ASSERT(!(block->status & BLOCK_CHANGED));
          if (block->hash_link->file == file) {
            /* We must skip blocks that will be changed. */
            if (block->status & BLOCK_FOR_UPDATE) {
              last_for_update = block;
              continue;
            }

            /*
              We must not free blocks in eviction (BLOCK_IN_EVICTION |
              BLOCK_IN_SWITCH) or blocks intended to be freed
              (BLOCK_REASSIGNED).
            */
            if (!(block->status &
                  (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH | BLOCK_REASSIGNED))) {
              HASH_LINK *next_hash_link = nullptr;
              my_off_t next_diskpos = 0;
              File next_file = 0;
              uint next_status = 0;
              uint hash_requests = 0;

              total_found++;
              found++;
              DBUG_ASSERT(found <= keycache->blocks_used);

              /*
                Register a request. This unlinks the block from the LRU
                ring and protects it against eviction. This is required
                by free_block().
              */
              reg_requests(keycache, block, 1);

              /*
                free_block() may need to wait for readers of the block.
                This is the moment where the other thread can move the
                'next' block from the chain. free_block() needs to wait
                if there are requests for the block pending.
              */
              if (next && (hash_requests = block->hash_link->requests)) {
                /* Copy values from the 'next' block and its hash_link. */
                next_status = next->status;
                next_hash_link = next->hash_link;
                next_diskpos = next_hash_link->diskpos;
                next_file = next_hash_link->file;
                DBUG_ASSERT(next == next_hash_link->block);
              }

              free_block(keycache, thread_var, block);
              /*
                If we had to wait and the state of the 'next' block
                changed, break the inner loop. 'next' may no longer be
                part of the current chain.

                We do not want to break the loop after every free_block(),
                not even only after waits. The chain might be quite long
                and contain blocks for many files. Traversing it again and
                again to find more blocks for this file could become quite
                inefficient.
              */
              if (next && hash_requests &&
                  ((next_status != next->status) ||
                   (next_hash_link != next->hash_link) ||
                   (next_file != next_hash_link->file) ||
                   (next_diskpos != next_hash_link->diskpos) ||
                   (next != next_hash_link->block)))
                break;
            } else {
              last_in_switch = block;
            }
          }
        } /* end for block in file_blocks */
      } while (found);

      /*
        If any clean block has been found, we may have waited for it to
        become free. In this case it could be possible that another clean
        block became dirty. This is possible if the write request existed
        before the flush started (BLOCK_FOR_UPDATE). Re-check the hashes.
      */
      if (total_found) goto restart;

      /*
        To avoid an infinite loop, wait until one of the blocks marked
        for update is updated.
      */
      if (last_for_update) {
        /* We did not wait. Block must not have changed status. */
        DBUG_ASSERT(last_for_update->status & BLOCK_FOR_UPDATE);
        wait_on_queue(&last_for_update->wqueue[COND_FOR_REQUESTED],
                      &keycache->cache_lock, thread_var);
        goto restart;
      }

      /*
        To avoid an infinite loop wait until one of the blocks marked
        for eviction is switched.
      */
      if (last_in_switch) {
        /* We did not wait. Block must not have changed status. */
        DBUG_ASSERT(last_in_switch->status &
                    (BLOCK_IN_EVICTION | BLOCK_IN_SWITCH | BLOCK_REASSIGNED));
        wait_on_queue(&last_in_switch->wqueue[COND_FOR_SAVED],
                      &keycache->cache_lock, thread_var);
        goto restart;
      }

    } /* if (! (type == FLUSH_KEEP || type == FLUSH_FORCE_WRITE)) */

  } /* if (keycache->disk_blocks > 0 */

err:
  if (cache != cache_buff) my_free(cache);
  if (last_errno) errno = last_errno; /* Return first error */
  return last_errno != 0;
}

/*
  Flush all blocks for a file to disk

  SYNOPSIS

    flush_key_blocks()
      keycache            pointer to a key cache data structure
      thread_var          pointer to thread specific variables
      file                handler for the file to flush to
      flush_type          type of the flush

  RETURN
    0   ok
    1  error
*/

int flush_key_blocks(KEY_CACHE *keycache, st_keycache_thread_var *thread_var,
                     File file, enum flush_type type) {
  int res = 0;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("keycache: %p", keycache));

  if (!keycache->key_cache_inited) return 0;

  mysql_mutex_lock(&keycache->cache_lock);
  /* While waiting for lock, keycache could have been ended. */
  if (keycache->disk_blocks > 0) {
    inc_counter_for_resize_op(keycache);
    res = flush_key_blocks_int(keycache, thread_var, file, type);
    dec_counter_for_resize_op(keycache);
  }
  mysql_mutex_unlock(&keycache->cache_lock);
  return res;
}

/*
  Flush all blocks in the key cache to disk.

  SYNOPSIS
    flush_all_key_blocks()
      keycache                  pointer to key cache root structure
      thread_var                pointer to thread specific variables

  DESCRIPTION

    Flushing of the whole key cache is done in two phases.

    1. Flush all changed blocks, waiting for them if necessary. Loop
    until there is no changed block left in the cache.

    2. Free all clean blocks. Normally this means free all blocks. The
    changed blocks were flushed in phase 1 and became clean. However we
    may need to wait for blocks that are read by other threads. While we
    wait, a clean block could become changed if that operation started
    before the resize operation started. To be safe we must restart at
    phase 1.

    When we can run through the changed_blocks and file_blocks hashes
    without finding a block any more, then we are done.

    Note that we hold keycache->cache_lock all the time unless we need
    to wait for something.

  RETURN
    0           OK
    != 0        Error
*/

static int flush_all_key_blocks(KEY_CACHE *keycache,
                                st_keycache_thread_var *thread_var) {
  BLOCK_LINK *block;
  uint total_found;
  uint found;
  uint idx;
  DBUG_TRACE;

  do {
    mysql_mutex_assert_owner(&keycache->cache_lock);
    total_found = 0;

    /*
      Phase1: Flush all changed blocks, waiting for them if necessary.
      Loop until there is no changed block left in the cache.
    */
    do {
      found = 0;
      /* Step over the whole changed_blocks hash array. */
      for (idx = 0; idx < CHANGED_BLOCKS_HASH; idx++) {
        /*
          If an array element is non-empty, use the first block from its
          chain to find a file for flush. All changed blocks for this
          file are flushed. So the same block will not appear at this
          place again with the next iteration. New writes for blocks are
          not accepted during the flush. If multiple files share the
          same hash bucket, one of them will be flushed per iteration
          of the outer loop of phase 1.
        */
        if ((block = keycache->changed_blocks[idx])) {
          found++;
          /*
            Flush dirty blocks but do not free them yet. They can be used
            for reading until all other blocks are flushed too.
          */
          if (flush_key_blocks_int(keycache, thread_var, block->hash_link->file,
                                   FLUSH_FORCE_WRITE))
            return 1;
        }
      }

    } while (found);

    /*
      Phase 2: Free all clean blocks. Normally this means free all
      blocks. The changed blocks were flushed in phase 1 and became
      clean. However we may need to wait for blocks that are read by
      other threads. While we wait, a clean block could become changed
      if that operation started before the resize operation started. To
      be safe we must restart at phase 1.
    */
    do {
      found = 0;
      /* Step over the whole file_blocks hash array. */
      for (idx = 0; idx < CHANGED_BLOCKS_HASH; idx++) {
        /*
          If an array element is non-empty, use the first block from its
          chain to find a file for flush. All blocks for this file are
          freed. So the same block will not appear at this place again
          with the next iteration. If multiple files share the
          same hash bucket, one of them will be flushed per iteration
          of the outer loop of phase 2.
        */
        if ((block = keycache->file_blocks[idx])) {
          total_found++;
          found++;
          if (flush_key_blocks_int(keycache, thread_var, block->hash_link->file,
                                   FLUSH_RELEASE))
            return 1;
        }
      }

    } while (found);

    /*
      If any clean block has been found, we may have waited for it to
      become free. In this case it could be possible that another clean
      block became dirty. This is possible if the write request existed
      before the resize started (BLOCK_FOR_UPDATE). Re-check the hashes.
    */
  } while (total_found);

#ifndef DBUG_OFF
  /* Now there should not exist any block any more. */
  for (idx = 0; idx < CHANGED_BLOCKS_HASH; idx++) {
    DBUG_ASSERT(!keycache->changed_blocks[idx]);
    DBUG_ASSERT(!keycache->file_blocks[idx]);
  }
#endif

  return 0;
}

/*
  Reset the counters of a key cache.

  SYNOPSIS
    reset_key_cache_counters()
    name       the name of a key cache
    key_cache  pointer to the key kache to be reset

  DESCRIPTION
   This procedure is used by process_key_caches() to reset the counters of all
   currently used key caches, both the default one and the named ones.

  RETURN
    0 on success (always because it can't fail)
*/

int reset_key_cache_counters(const char *name MY_ATTRIBUTE((unused)),
                             KEY_CACHE *key_cache) {
  DBUG_TRACE;
  if (!key_cache->key_cache_inited) {
    DBUG_PRINT("info", ("Key cache %s not initialized.", name));
    return 0;
  }
  DBUG_PRINT("info", ("Resetting counters for key cache %s.", name));

  key_cache->global_blocks_changed = 0;   /* Key_blocks_not_flushed */
  key_cache->global_cache_r_requests = 0; /* Key_read_requests */
  key_cache->global_cache_read = 0;       /* Key_reads */
  key_cache->global_cache_w_requests = 0; /* Key_write_requests */
  key_cache->global_cache_write = 0;      /* Key_writes */
  return 0;
}

#if !defined(DBUG_OFF)
#define F_B_PRT(_f_, _v_) DBUG_PRINT("assert_fail", (_f_, _v_))

static int fail_block(BLOCK_LINK *block) {
  F_B_PRT("block->next_used:    %p\n", block->next_used);
  F_B_PRT("block->prev_used:    %p\n", block->prev_used);
  F_B_PRT("block->next_changed: %p\n", block->next_changed);
  F_B_PRT("block->prev_changed: %p\n", block->prev_changed);
  F_B_PRT("block->hash_link:    %p\n", block->hash_link);
  F_B_PRT("block->status:       %u\n", block->status);
  F_B_PRT("block->length:       %u\n", block->length);
  F_B_PRT("block->offset:       %u\n", block->offset);
  F_B_PRT("block->requests:     %u\n", block->requests);
  F_B_PRT("block->temperature:  %u\n", block->temperature);
  return 0; /* Let the assert fail. */
}

static int fail_hlink(HASH_LINK *hlink) {
  F_B_PRT("hlink->next:    %p\n", hlink->next);
  F_B_PRT("hlink->prev:    %p\n", hlink->prev);
  F_B_PRT("hlink->block:   %p\n", hlink->block);
  F_B_PRT("hlink->diskpos: %lu\n", (ulong)hlink->diskpos);
  F_B_PRT("hlink->file:    %d\n", hlink->file);
  return 0; /* Let the assert fail. */
}

static int cache_empty(KEY_CACHE *keycache) {
  char buf[512];
  int errcnt = 0;
  int idx;
  if (keycache->disk_blocks <= 0) return 1;
  for (idx = 0; idx < keycache->disk_blocks; idx++) {
    BLOCK_LINK *block = keycache->block_root + idx;
    if (block->status || block->requests || block->hash_link) {
      snprintf(buf, sizeof(buf) - 1, "block index: %u", idx);
      my_message_local(INFORMATION_LEVEL, EE_DEBUG_INFO, buf);
      fail_block(block);
      errcnt++;
    }
  }
  for (idx = 0; idx < keycache->hash_links; idx++) {
    HASH_LINK *hash_link = keycache->hash_link_root + idx;
    if (hash_link->requests || hash_link->block) {
      snprintf(buf, sizeof(buf) - 1, "hash_link index: %u", idx);
      my_message_local(INFORMATION_LEVEL, EE_DEBUG_INFO, buf);
      fail_hlink(hash_link);
      errcnt++;
    }
  }
  if (errcnt) {
    snprintf(buf, sizeof(buf) - 1, "blocks: %d  used: %lu",
             keycache->disk_blocks, keycache->blocks_used);
    my_message_local(INFORMATION_LEVEL, EE_DEBUG_INFO, buf);
    snprintf(buf, sizeof(buf) - 1, "hash_links: %d  used: %d",
             keycache->hash_links, keycache->hash_links_used);
    my_message_local(INFORMATION_LEVEL, EE_DEBUG_INFO, buf);
  }
  return !errcnt;
}
#endif
