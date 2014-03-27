/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*****************************************************************************/
/* File Implementing the shared memory system for Hack.
 *
 * THIS CODE ONLY WORKS WITH HACK, IT MAY LOOK LIKE A GENERIC ATOMIC
 * HASHTABLE FOR OCAML: IT IS NOT!
 * BUT ... YOU WERE GOING TO SAY BUT? BUT ...
 * THERE IS NO BUT! DONNY YOU'RE OUT OF YOUR ELEMENT!
 *
 * The lock-free data structures implemented here only work because of how 
 * the Hack phases are synchronized. 
 *
 * There are 3 kinds of storage implemented in this file.
 * I) The global storage. Used by the master to efficiently transfer a blob
 *    of data to the workers. This is used to share an environment in 
 *    read-only mode with all the workers.
 *    The master stores, the workers read.
 *
 * II) The dependency table. It's a hashtable that contains all the
 *    dependencies between Hack objects. It is filled concurrently by
 *    the workers.
 *
 * II) The Hashtable.
 *     The operations implemented, and their limitations:
 * 
 *    -) Concurrent writes: SUPPORTED
 *       As long as its not interleaved with any other operation 
 *       (other than mem)!
 *
 *    -) Concurrent reads: SUPPORTED
 *       As long as they are no concurrent writers.
 *
 *    -) Concurrent removes: NOT SUPPORTED
 *       Only the master can remove.
 */
/*****************************************************************************/

#include <caml/memory.h>
#include <caml/alloc.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <stdint.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/syscall.h>

#define GIG (1024l * 1024l * 1024l)

/* Convention: .*_B = Size in bytes. */

/* Size of the "global storage". */
#define GLOBAL_SIZE_B   GIG

/* Used for the dependency hashtable */
#define DEP_POW         23
#define DEP_SIZE        (1 << DEP_POW)
#define DEP_SIZE_B      (DEP_SIZE * sizeof(value))

/* Used for the shared hashtable */
#define HASHTBL_POW     23
#define HASHTBL_SIZE    (1 << HASHTBL_POW)
#define HASHTBL_SIZE_B  (HASHTBL_SIZE * sizeof(helt_t))

/* Size of where we allocate shared objects. */
#define HEAP_SIZE       (8 * GIG)
#define Get_size(x)     (((size_t*)(x))[-1])
#define Get_buf_size(x) (((size_t*)(x))[-1] + sizeof(size_t))
#define Get_buf(x)      (x - sizeof(size_t))

/* The total size of the shared memory.
 * Most of it is going to remain virtual.
 */
#define SHARED_MEM_SIZE (GLOBAL_SIZE_B + DEP_SIZE_B + HASHTBL_SIZE_B +\
                         HEAP_SIZE)

/* Too lazy to use getconf */
#define CACHE_LINE_SIZE (1 << 6)
#define CACHE_MASK      (~(CACHE_LINE_SIZE - 1))
#define ALIGNED(x)      ((x + CACHE_LINE_SIZE - 1) & CACHE_MASK)

/*****************************************************************************/
/* Types */
/*****************************************************************************/

/* Cells of the Hashtable */
typedef struct {
  unsigned long hash;
  char* addr;
} helt_t;

/*****************************************************************************/
/* Globals */
/*****************************************************************************/

/* The location of the shared memory */
static char* shared_mem;

/* ENCODING: The first element is the size stored in bytes, the rest is 
 * the data. The size is set to zero when the storage is empty.
 */
static value* global_storage;

/* ENCODING: 
 * The highest 2 bits are unused.
 * The next 31 bits encode the key the lower 31 bits the value.
 */
static uint64_t* deptbl;

/* The hashtable containing the shared values. */
static helt_t* hashtbl;
static int* hcounter;   // the number of slots taken in the table

/* A counter increasing globally across all forks. */
static uintptr_t* counter;
static uintptr_t early_counter = 1;

/* The top of the heap */
static char** heap;

/* Useful to add assertions */
static pid_t master_pid;
static pid_t my_pid;

/* Where the heap started (bottom) */
static char* heap_init;

/* The size of the heap after initialization of the server */
static size_t heap_init_size = 0;

/* The size of a page (memory page) */
static int page_size;

/*****************************************************************************/
/* Given a pointer to the shared memory address space, initializes all
 * the globals that live in shared memory.
 */
/*****************************************************************************/
static void init_shared_globals(char* mem) {
  int page_size = getpagesize();
  char* bottom  = mem;

  /* We keep all the small objects in the first page.
   * There are on different cache lines because we modify them atomically.
   */

  /* BEGINING OF THE FIRST PAGE */
  /* The pointer to the top of the heap.
   * We will atomically increment *heap every time we want to allocate.
   */
  heap = (char**)mem;
  assert(CACHE_LINE_SIZE >= sizeof(char*));

  // The number of elements in the hashtable
  hcounter = (int*)(mem + CACHE_LINE_SIZE);
  *hcounter = 0;

  counter = (uintptr_t*)(mem + 2*CACHE_LINE_SIZE);
  *counter = early_counter + 1;

  mem += page_size;
  // Just checking that the page is large enough.
  assert(page_size > CACHE_LINE_SIZE + sizeof(int));
  /* END OF THE FIRST PAGE */

  /* Global storage initialization */
  global_storage = (value*)mem;
  // Initial size is zero
  global_storage[0] = 0;
  mem += GLOBAL_SIZE_B;

  /* Dependencies */
  deptbl = (uint64_t*)mem;
  mem += DEP_SIZE_B;

  /* Hashtable */
  hashtbl = (helt_t*)mem;
  mem += HASHTBL_SIZE_B;

  /* Heap */
  heap_init = mem;
  *heap = mem;

  // Checking that we did the maths correctly.
  assert(mem + HEAP_SIZE == bottom + SHARED_MEM_SIZE + page_size);
}

/*****************************************************************************/
/* Sets CPU and IO priorities. */
/*****************************************************************************/

// glibc refused to add ioprio_set, sigh.
// https://sourceware.org/bugzilla/show_bug.cgi?id=4464
#define IOPRIO_CLASS_SHIFT 13
#define IOPRIO_PRIO_VALUE(cl, dat) (((cl) << IOPRIO_CLASS_SHIFT) | (dat))
#define IOPRIO_WHO_PROCESS 1
#define IOPRIO_CLASS_IDLE 3

static void set_priorities() {
  // Downgrade to lowest IO priority. We fork a process for each CPU, which
  // during parsing can slam the disk so hard that the system becomes
  // unresponsive. While it's unclear why the Linux IO scheduler can't deal with
  // this better, increasing our startup time in return for a usable system
  // while we start up is the right tradeoff. (Especially in Facebook's
  // configuration, where hh_server is often started up in the background well
  // before the user needs hh_client, so our startup time often doesn't matter
  // at all!)
  //
  // No need to check the return value, if we failed then whatever.
  syscall(
    SYS_ioprio_set,
    IOPRIO_WHO_PROCESS,
    my_pid,
    IOPRIO_PRIO_VALUE(IOPRIO_CLASS_IDLE, 7)
  );

  // Don't slam the CPU either, though this has much less tendency to make the
  // system totally unresponsive so we don't need to lower all the way.
  nice(10);
}

/*****************************************************************************/
/* Must be called by the master BEFORE forking the workers! */
/*****************************************************************************/
void hh_shared_init() {
  /* MAP_NORESERVE is because we want a lot more virtual memory than what
   * we are actually going to use.
   */
  int flags = MAP_SHARED | MAP_ANONYMOUS | MAP_NORESERVE;
  int prot  = PROT_READ  | PROT_WRITE;

  page_size = getpagesize();

  shared_mem = 
    (char*)mmap(NULL, page_size + SHARED_MEM_SIZE, prot, flags, 0, 0);

  if(shared_mem == MAP_FAILED) {
    printf("Error initializing: %s\n", strerror(errno));
    exit(2);
  }

  // Keeping the pids around to make asserts.
  master_pid = getpid();
  my_pid = master_pid;

  init_shared_globals(shared_mem);

  // Uninstall ocaml's segfault handler. It's supposed to throw an exception on
  // stack overflow, but we don't actually handle that exception, so what
  // happens in practice is we terminate at toplevel with an unhandled exception
  // and a useless ocaml backtrace. A core dump is actually more useful. Sigh.
  struct sigaction sigact = {};
  sigact.sa_handler = SIG_DFL;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = 0;
  sigaction(SIGSEGV, &sigact, NULL);

  set_priorities();
}

/* Must be called by every worker before any operation is performed */
void hh_worker_init() {
  my_pid = getpid();
}

/*****************************************************************************/
/* Counter
 *
 * Provides a counter intended to be increasing over the lifetime of the program
 * including all forks. Uses a global variable until hh_shared_init is called,
 * so it's safe to use in the early init stages of the program (as long as you
 * fork after hh_shared_init of course). Wraps around at the maximum value of an
 * ocaml int, which is something like 30 or 62 bits on 32 and 64-bit
 * architectures respectively.
 */
/*****************************************************************************/

value hh_counter_next() {
  CAMLparam0();
  CAMLlocal1(result);

  uintptr_t v;
  if (counter) {
    v = __sync_fetch_and_add(counter, 1);
  } else {
    v = ++early_counter;
  }

  result = Val_long(v % Max_long); // Wrap around.
  CAMLreturn(result);
}

/*****************************************************************************/
/* Global storage */
/*****************************************************************************/

void hh_shared_store(value data) {
  size_t size = caml_string_length(data);

  assert(my_pid == master_pid);                  // only the master can store
  assert(global_storage[0] == 0);                // Is it clear?
  assert(size < GLOBAL_SIZE_B - sizeof(value));  // Do we have enough space?

  global_storage[0] = size;
  memcpy(&global_storage[1], &Field(data, 0), size);
}

/*****************************************************************************/
/* We are allocating ocaml values. The OCaml GC must know about them.
 * caml_alloc_string might trigger the GC, when that happens, the GC needs
 * to scan the stack to find the OCaml roots. The macros CAMLparam0 and
 * CAMLlocal1 register the roots.
 */
/*****************************************************************************/

value hh_shared_load() {
  CAMLparam0();
  CAMLlocal1(result);

  size_t size = global_storage[0];
  assert(size != 0);
  result = caml_alloc_string(size);
  memcpy(&Field(result, 0), &global_storage[1], size);

  CAMLreturn(result);
}

void hh_shared_clear() {
  assert(my_pid == master_pid);
  global_storage[0] = 0;
}

/*****************************************************************************/
/* Dependencies */
/*****************************************************************************/
/* This code is very perf sensitive, please check the performance before
 * modifying.
 * The table contains key/value bindings encoded in a word.
 * The higher bits represent the key, the lower ones the value.
 * Each key/value binding is unique, but a key can have multiple value 
 * bound to it.
 * Concretely, if you try to add a key/value pair that is already in the table
 * the data structure is left unmodified.
 * If you try to add a key bound to a new value, the binding is added, the
 * old binding is not removed.
 */
/*****************************************************************************/

void hh_add_dep(value ocaml_dep) {
  unsigned long dep  = Long_val(ocaml_dep);
  unsigned long hash = dep >> 31;
  unsigned long slot = hash & (DEP_SIZE - 1);
  
  while(1) {
    /* It considerably speeds things up to do a normal load before trying using
     * an atomic operation.
     */
    uint64_t slot_val = deptbl[slot];

    // The binding exists, done!
    if(slot_val == dep)
      return;
    
    // The slot is free, let's try to take it.
    if(slot_val == 0) {
      // See comments in hh_add about its similar construction here.
      if(__sync_bool_compare_and_swap(&deptbl[slot], 0, dep)) {
        return;
      }

      if(deptbl[slot] == dep) {
        return;
      }
    }

    slot = (slot + 1) & (DEP_SIZE - 1);
  }
}

/* Given a key, returns the list of values bound to it. */
value hh_get_dep(value dep) {
  CAMLparam1(dep);
  CAMLlocal2(result, cell);

  unsigned long hash = Long_val(dep);
  unsigned long slot = hash & (DEP_SIZE - 1);
  
  result = Val_int(0); // The empty list

  while(1) {
    if(deptbl[slot] == 0) {
      break;
    }
    if(deptbl[slot] >> 31 == hash) {
      cell = caml_alloc_tuple(2);
      Field(cell, 0) = Val_long(deptbl[slot] & ((1l << 31) - 1));
      Field(cell, 1) = result;
      result = cell;
    }
    slot = (slot + 1) & (DEP_SIZE - 1);
  }

  CAMLreturn(result);
}

/*****************************************************************************/
/* Garbage collector */
/*****************************************************************************/

/*****************************************************************************/
/* Must be called after the hack server is done initializing.
 * We keep the original size of the heap to estimate how often we should
 * garbage collect.
 */
/*****************************************************************************/
void hh_call_after_init() {
  heap_init_size = (uintptr_t)*heap - (uintptr_t)heap_init;
}

/*****************************************************************************/
/* We compact the heap when it gets twice as large as its initial size.
 * Step one, copy the live values in a new heap.
 * Step two, memcopy the values back into the shared heap.
 * We could probably use something smarter, but this is fast enough.
 *
 * The collector should only be called by the master.
 */
/*****************************************************************************/
void hh_collect() {
  int flags       = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
  int prot        = PROT_READ | PROT_WRITE;
  char* dest;
  size_t mem_size = 0;
  char* tmp_heap;

  assert(heap_init_size >= 0);

  if(*heap < heap_init + 2 * heap_init_size) {
    // We have not grown passed twice the size of the initial size
    return;
  }

  tmp_heap = (char*)mmap(NULL, HEAP_SIZE, prot, flags, 0, 0);
  dest = tmp_heap;

  if(tmp_heap == MAP_FAILED) {
    printf("Error while collecting: %s\n", strerror(errno));
    exit(2);
  }

  assert(my_pid == master_pid); // Comes from the master

  // Walking the table
  int i;
  for(i = 0; i < HASHTBL_SIZE; i++) {
    if(hashtbl[i].addr != NULL) { // Found a non empty slot
      size_t bl_size      = Get_buf_size(hashtbl[i].addr);
      size_t aligned_size = ALIGNED(bl_size);
      char* addr          = Get_buf(hashtbl[i].addr);

      memcpy(dest, addr, bl_size);
      // This is where the data ends up after the copy
      hashtbl[i].addr = heap_init + mem_size + sizeof(size_t);
      dest     += aligned_size;
      mem_size += aligned_size;
    }
  }

  // Copying the result back into shared memory
  memcpy(heap_init, tmp_heap, mem_size);
  *heap = heap_init + mem_size;

  if(munmap(tmp_heap, HEAP_SIZE) == -1) {
    printf("Error while collecting: %s\n", strerror(errno));
    exit(2);    
  }
}

/*****************************************************************************/
/* Allocates in the shared heap.
 * The chunks are cache aligned.
 * The word before the chunk address contains the size of the chunk in bytes.
 * The function returns a pointer to the data (the size can be accessed by
 * looking at the address: chunk - sizeof(size_t)).
 */
/*****************************************************************************/

static char* hh_alloc(size_t size) {
  size_t slot_size  = ALIGNED(size + sizeof(size_t));
  char* chunk       = __sync_fetch_and_add(heap, slot_size);
  *((size_t*)chunk) = size;
  return (chunk + sizeof(size_t));
}

/*****************************************************************************/
/* Allocates an ocaml value in the shared heap. 
 * The values can only be ocaml strings. It returns the address of the 
 * allocated chunk.
 */
/*****************************************************************************/
static char* hh_store_ocaml(value data) {
  size_t data_size = caml_string_length(data);
  char* addr = hh_alloc(data_size);
  memcpy(addr, String_val(data), data_size);
  return addr;
}

/*****************************************************************************/
/* Given an OCaml string, returns the 8 first bytes in an unsigned long.
 * The key is generated using MD5, but we only use the first 8 bytes because
 * it allows us to use atomic operations.
 */
/*****************************************************************************/
static unsigned long get_hash(value key) {
  return *((unsigned long*)String_val(key));
}

/*****************************************************************************/
/* Writes the data in one of the slots of the hashtable. There might be
 * concurrent writers, when that happens, the first writer wins.
 */
/*****************************************************************************/
static void write_at(unsigned int slot, value data) {
  if(hashtbl[slot].addr == NULL &&
     __sync_bool_compare_and_swap(&(hashtbl[slot].addr), NULL, 1)) {
    hashtbl[slot].addr = hh_store_ocaml(data);
  }
}

/*****************************************************************************/
/* Adds a key value to the hashtable. This code is perf sensitive, please
 * check the perf before modifying.
 */
/*****************************************************************************/
void hh_add(value key, value data) {
  unsigned long hash = get_hash(key);
  unsigned int slot = hash & (HASHTBL_SIZE - 1);

  while(1) {
    unsigned long slot_hash = hashtbl[slot].hash;

    if(slot_hash == hash) {
      write_at(slot, data);
      return;
    }

    if(slot_hash == 0) {
      // We think we might have a free slot, try to atomically grab it.
      if(__sync_bool_compare_and_swap(&(hashtbl[slot].hash), 0, hash)) {
        unsigned long size = __sync_fetch_and_add(hcounter, 1);
        assert(size < HASHTBL_SIZE);
        write_at(slot, data);
        return;
      }

      // Grabbing it failed -- why? If someone else inserted the data we were
      // about to, we are done (don't double-insert). Otherwise, keep going.
      // Note that this read relies on the __sync call above preventing the
      // compiler from caching the value read out of memory. (And of course
      // isn't safe on any arch that requires memory barriers.)
      if(hashtbl[slot].hash == hash) {
        // FIXME: there is a race here. The data may not actually be written by
        // the time we return here, and even the sigil value "1" may not be
        // written into the address by the winning thread. If this thread
        // manages to call hh_mem on this key before the winning thread can
        // write the sigil "1", things will be broken since the data we just
        // wrote will be missing. Want to more carefully think out the right
        // fix and need to commit a fix for a much worse race, so leaving this
        // here for now -- this thread has to get all the way back into hh_mem
        // before the other thread executes the 37 instructions it takes to
        // write the sigil, so I'm not super worried.
        return;
      }
    }

    slot = (slot + 1) & (HASHTBL_SIZE - 1);
  }
}

/*****************************************************************************/
/* Finds the slot corresponding to the key in a hash table. The returned slot
 * is either free or points to the key.
 */
/*****************************************************************************/
static unsigned int find_slot(value key) {
  unsigned long hash = get_hash(key);
  unsigned int slot = hash & (HASHTBL_SIZE - 1);

  while(1) {
    if(hashtbl[slot].hash == hash) {
      return slot;
    }
    if(hashtbl[slot].hash == 0) {
      return slot;
    }
    slot = (slot + 1) & (HASHTBL_SIZE - 1);
  }
}

/*****************************************************************************/
/* Returns true if the key is present. We need to check both the hash and
 * the address of the data. This is due to the fact that we remove by setting
 * the address slot to NULL (we never remove a hash from the table, outside
 * of garbage collection).
 */
/*****************************************************************************/
value hh_mem(value key) {
  unsigned int slot = find_slot(key);
  if(hashtbl[slot].hash == get_hash(key) &&
     hashtbl[slot].addr != NULL) {
    // The data is currently in the process of being written, wait until it
    // actually is ready to be used before returning.
    while (hashtbl[slot].addr == (char*)1) {
      asm volatile("pause" : : : "memory");
    }
    return Val_bool(1);
  }
  return Val_bool(0);
}

/*****************************************************************************/
/* Returns the value associated to a given key. The key MUST be present. */
/*****************************************************************************/
value hh_get(value key) {
  CAMLparam1(key);
  CAMLlocal1(result);

  unsigned int slot = find_slot(key);
  assert(hashtbl[slot].hash == get_hash(key));
  size_t size = *(size_t*)(hashtbl[slot].addr - sizeof(size_t));
  result = caml_alloc_string(size);
  memcpy(String_val(result), hashtbl[slot].addr, size);

  CAMLreturn(result);
}

/*****************************************************************************/
/* Moves the data associated to key1 to key2.
 * key1 must be present.
 * key2 must be free.
 * Only the master can perform this operation.
 */
/*****************************************************************************/
void hh_move(value key1, value key2) {
  unsigned int slot1 = find_slot(key1);
  unsigned int slot2 = find_slot(key2);

  assert(my_pid == master_pid);
  assert(hashtbl[slot1].hash == get_hash(key1));
  assert(hashtbl[slot2].addr == NULL);
  hashtbl[slot2].hash = get_hash(key2);
  hashtbl[slot2].addr = hashtbl[slot1].addr;
  hashtbl[slot1].addr = NULL;
}

/*****************************************************************************/
/* Removes a key from the hash table.
 * Only the master can perform this operation.
 */
/*****************************************************************************/
void hh_remove(value key) {
  unsigned int slot = find_slot(key);
  
  assert(my_pid == master_pid);
  assert(hashtbl[slot].hash == get_hash(key));
  hashtbl[slot].addr = NULL;
}

/*****************************************************************************/
/* Returns a copy of the content of a file in an ocaml string.
 * This code should be very tolerant to failure. At any given time, the
 * file could be modified, when that happens, we don't want to fail, we 
 * return the empty string instead.
 */
/*****************************************************************************/

value hh_read_file(value filename) {
  CAMLparam1(filename);
  CAMLlocal1(result);

  int fd;
  struct stat sb;
  char* memblock;
    
  fd = open(String_val(filename), O_RDONLY);
  if(fd == -1) {
    result = caml_alloc_string(0);
  }
  else if(fstat(fd, &sb) == -1) {
    result = caml_alloc_string(0);
    close(fd);
  }
  else if((memblock = 
           (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0))
          == MAP_FAILED) {
    result = caml_alloc_string(0);
    close(fd);
  }
  else {
    result = caml_alloc_string(sb.st_size);
    memcpy(String_val(result), memblock, sb.st_size);
    munmap(memblock, sb.st_size);
    close(fd);
  }
    
  CAMLreturn(result);
}
