/**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

#include "hh_shared.h"
/* For some reason this header file is not on path in OSS builds.
 * But we only lose the ability to trim the OCaml heap after a GC
 */
#if __has_include("malloc.h")
#  define MALLOC_TRIM
#  include <malloc.h>
#endif

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
 * There are 2 kinds of storage implemented in this file.
 * I) The global storage. Used by the master to efficiently transfer a blob
 *    of data to the workers. This is used to share an environment in
 *    read-only mode with all the workers.
 *    The master stores, the workers read.
 *    Only concurrent reads allowed. No concurrent write/read and write/write.
 *    There are a few different OCaml modules that act as interfaces to this
 *    global storage. They all use the same area of memory, so only one can be
 *    active at any one time. The first word indicates the size of the global
 *    storage currently in use; callers are responsible for setting it to zero
 *    once they are done.
 *
 * II) The hashtable that maps string keys to string values. (The strings
 *    are really serialized / marshalled representations of OCaml structures.)
 *    Key observation of the table is that data with the same key are
 *    considered equivalent, and so you can arbitrarily get any copy of it;
 *    furthermore if data is missing it can be recomputed, so incorrectly
 *    saying data is missing when it is being written is only a potential perf
 *    loss. Note that "equivalent" doesn't necessarily mean "identical", e.g.,
 *    two alpha-converted types are "equivalent" though not literally byte-
 *    identical. (That said, I'm pretty sure the Hack typechecker actually does
 *    always write identical data, but the hashtable doesn't need quite that
 *    strong of an invariant.)
 *
 *    The operations implemented, and their limitations:
 *
 *    -) Concurrent writes: SUPPORTED
 *       One will win and the other will get dropped on the floor. There is no
 *       way to tell which happened. Only promise is that after a write, the
 *       one thread which did the write will see data in the table (though it
 *       may be slightly different data than what was written, see above about
 *       equivalent data).
 *
 *    -) Concurrent reads: SUPPORTED
 *       If interleaved with a concurrent write, the read will arbitrarily
 *       say that there is no data at that slot or return the entire new data
 *       written by the concurrent writer.
 *
 *    -) Concurrent removes: NOT SUPPORTED
 *       Only the master can remove, and can only do so if there are no other
 *       concurrent operations (reads or writes).
 *
 *    Since the values are variably sized and can get quite large, they are
 *    stored separately from the hashes in a garbage-collected heap.
 *
 * Both II and III resolve hash collisions via linear probing.
 */
/*****************************************************************************/

/* For printing uint64_t
 * http://jhshi.me/2014/07/11/print-uint64-t-properly-in-c/index.html */
#define __STDC_FORMAT_MACROS

/* define CAML_NAME_SPACE to ensure all the caml imports are prefixed with
 * 'caml_' */
#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/unixsupport.h>
#include <caml/intext.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <fcntl.h>
#  include <pthread.h>
#  include <signal.h>
#  include <stdint.h>
#  include <stdio.h>
#  include <string.h>
#  include <sys/errno.h>
#  include <sys/mman.h>
#  include <sys/resource.h>
#  include <sys/stat.h>
#  include <sys/syscall.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include <inttypes.h>
#include <lz4.h>
#include <sys/time.h>
#include <time.h>
#include <zstd.h>

#include "dictionary_data.h"

// Some OCaml utility functions (introduced only in 4.12.0)
//
// TODO(hverr): Remove these when we move to 4.12.0
static value hh_shared_caml_alloc_some(value v) {
  CAMLparam1(v);
  value some = caml_alloc_small(1, 0);
  Store_field(some, 0, v);
  CAMLreturn(some);
}
#  define Val_none Val_int(0)

#include "hh_assert.h"

#define UNUSED(x) \
    ((void)(x))
#define UNUSED1 UNUSED
#define UNUSED2(a, b) \
    (UNUSED(a), UNUSED(b))
#define UNUSED3(a, b, c) \
    (UNUSED(a), UNUSED(b), UNUSED(c))
#define UNUSED4(a, b, c, d) \
    (UNUSED(a), UNUSED(b), UNUSED(c), UNUSED(d))
#define UNUSED5(a, b, c, d, e) \
    (UNUSED(a), UNUSED(b), UNUSED(c), UNUSED(d), UNUSED(e))


// Ideally these would live in a handle.h file but our internal build system
// can't support that at the moment. These are shared with handle_stubs.c
#ifdef _WIN32
#  define Val_handle(fd) (win_alloc_handle(fd))
#else
#  define Handle_val(fd) (Long_val(fd))
#  define Val_handle(fd) (Val_long(fd))
#endif


#define HASHTBL_WRITE_IN_PROGRESS ((heap_entry_t*)1)

/****************************************************************************
 * Quoting the linux manpage: memfd_create() creates an anonymous file
 * and returns a file descriptor that refers to it. The file behaves
 * like a regular file, and so can be modified, truncated,
 * memory-mapped, and so on. However, unlike a regular file, it lives
 * in RAM and has a volatile backing storage. Once all references to
 * the file are dropped, it is automatically released. Anonymous
 * memory is used for all backing pages of the file. Therefore, files
 * created by memfd_create() have the same semantics as other
 * anonymous memory allocations such as those allocated using mmap(2)
 * with the MAP_ANONYMOUS flag. The memfd_create() system call first
 * appeared in Linux 3.17.
 ****************************************************************************/
#ifdef __linux__
#  define MEMFD_CREATE 1
   // glibc only added support for memfd_create in version 2.27.
#  ifndef MFD_CLOEXEC
     // Linux version for the architecture must support syscall
     // memfd_create
#    ifndef SYS_memfd_create
#      if defined(__x86_64__)
#        define SYS_memfd_create 319
#      elif defined(__aarch64__)
#        define SYS_memfd_create 385
#      else
#        error "hh_shared.c requires an architecture that supports memfd_create"
#      endif //#if defined(__x86_64__)
#    endif //#ifndef SYS_memfd_create

#    include <asm/unistd.h>

  /* Originally this function would call uname(), parse the linux
   * kernel release version and make a decision based on whether
   * the kernel was >= 3.17 or not. However, syscall will return -1
   * with an strerr(errno) of "Function not implemented" if the
   * kernel is < 3.17, and that's good enough.
   */
  static int memfd_create(const char *name, unsigned int flags) {
    return syscall(SYS_memfd_create, name, flags);
  }
#  endif // #ifndef MFD_CLOEXEC
#endif //#ifdef __linux__

#ifndef MAP_NORESERVE
  // This flag was unimplemented in FreeBSD and then later removed
#  define MAP_NORESERVE 0
#endif

// The following 'typedef' won't be required anymore
// when dropping support for OCaml < 4.03
#ifdef __MINGW64__
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#endif

#ifdef _WIN32
static int win32_getpagesize(void) {
  SYSTEM_INFO siSysInfo;
  GetSystemInfo(&siSysInfo);
  return siSysInfo.dwPageSize;
}
#  define getpagesize win32_getpagesize
#endif


/*****************************************************************************/
/* API to shmffi */
/*****************************************************************************/

extern void shmffi_init(void* mmap_address, size_t file_size, ssize_t max_evictable_bytes);
extern void shmffi_attach(void* mmap_address, size_t file_size);
extern value shmffi_add(_Bool evictable, uint64_t hash, value data);
extern value shmffi_mem(uint64_t hash);
extern value shmffi_get_and_deserialize(uint64_t hash);
extern value shmffi_mem_status(uint64_t hash);
extern value shmffi_get_size(uint64_t hash);
extern void shmffi_move(uint64_t hash1, uint64_t hash2);
extern value shmffi_remove(uint64_t hash);
extern value shmffi_allocated_bytes();
extern value shmffi_num_entries();

extern value shmffi_add_raw(uint64_t hash, value data);
extern value shmffi_get_raw(uint64_t hash);
extern value shmffi_deserialize_raw(value data);
extern value shmffi_serialize_raw(value data);

/*****************************************************************************/
/* Config settings (essentially constants, so they don't need to live in shared
 * memory), initialized in hh_shared_init */
/*****************************************************************************/

/* Convention: .*_b = Size in bytes. */

static size_t global_size_b;
static size_t global_size;
static size_t heap_size;
static size_t hash_table_pow;
static size_t shm_use_sharded_hashtbl;
static ssize_t shm_cache_size_b;

/* Used for the shared hashtable */
static uint64_t hashtbl_size;
static size_t hashtbl_size_b;

/* Used for worker-local data */
static size_t locals_size_b;

typedef enum {
  KIND_STRING = 1,
  KIND_SERIALIZED = !KIND_STRING
} storage_kind;

/* Too lazy to use getconf */
#define CACHE_LINE_SIZE (1 << 6)

#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define CACHE_ALIGN(x)          ALIGN(x,CACHE_LINE_SIZE)

/* Align heap entries on 64-bit boundaries */
#define HEAP_ALIGN(x)           ALIGN(x,8)

/* Fix the location of our shared memory so we can save and restore the
 * hashtable easily */
#ifdef _WIN32
/* We have to set differently our shared memory location on Windows. */
#  define SHARED_MEM_INIT ((char *) 0x48047e00000ll)
#elif defined __aarch64__
/* CentOS 7.3.1611 kernel does not support a full 48-bit VA space, so choose a
 * value low enough that the 100 GB's mmapped in do not interfere with anything
 * growing down from the top. 1 << 36 works. */
#  define SHARED_MEM_INIT ((char *) 0x1000000000ll)
#  define SHARDED_HASHTBL_MEM_ADDR ((char *) 0x2000000000ll)
#  define SHARDED_HASHTBL_MEM_SIZE ((size_t)100 * 1024 * 1024 * 1024)
#else
#  define SHARED_MEM_INIT ((char *) 0x500000000000ll)
#  define SHARDED_HASHTBL_MEM_ADDR ((char *) 0x510000000000ll)
#  define SHARDED_HASHTBL_MEM_SIZE ((size_t)200 * 1024 * 1024 * 1024)
#endif

/* As a sanity check when loading from a file */
static const uint64_t MAGIC_CONSTANT = 0xfacefacefaceb000ull;

/* The VCS identifier (typically a git hash) of the build */
extern const char* const BuildInfo_kRevision;

/*****************************************************************************/
/* Types */
/*****************************************************************************/

/* Per-worker data which can be quickly updated non-atomically. Will be placed
 * in cache-aligned array in the first few pages of shared memory, indexed by
 * worker id. */
typedef struct {
  uint64_t counter;
} local_t;

// Every heap entry starts with a 64-bit header with the following layout:
//
//  6                                3 3  3                                0 0
//  3                                3 2  1                                1 0
// +----------------------------------+-+-----------------------------------+-+
// |11111111 11111111 11111111 1111111|0| 11111111 11111111 11111111 1111111|1|
// +----------------------------------+-+-----------------------------------+-+
// |                                  | |                                   |
// |                                  | |                                   * 0 tag
// |                                  | |
// |                                  | * 31-1 uncompressed size (0 if uncompressed)
// |                                  |
// |                                  * 32 kind (0 = serialized, 1 = string)
// |
// * 63-33 size of heap entry
//
// The tag bit is always 1 and is used to differentiate headers from pointers
// during garbage collection (see hh_collect).
typedef uint64_t hh_header_t;

#define Entry_size(x) ((x) >> 33)
#define Entry_kind(x) (((x) >> 32) & 1)
#define Entry_uncompressed_size(x) (((x) >> 1) & 0x7FFFFFFF)
#define Heap_entry_total_size(header) sizeof(heap_entry_t) + Entry_size(header)

/* Shared memory structures. hh_shared.h typedefs this to heap_entry_t. */
typedef struct {
  hh_header_t header;
  char data[];
} heap_entry_t;

/* Cells of the Hashtable */
typedef struct {
  uint64_t hash;
  heap_entry_t* addr;
} helt_t;

/*****************************************************************************/
/* Globals */
/*****************************************************************************/

/* Total size of allocated shared memory */
static size_t shared_mem_size = 0;

/* Beginning of shared memory */
static char* shared_mem = NULL;

/* ENCODING: The first element is the size stored in bytes, the rest is
 * the data. The size is set to zero when the storage is empty.
 */
static value* global_storage = NULL;

/* The hashtable containing the shared values. */
static helt_t* hashtbl = NULL;
/* The number of nonempty slots in the hashtable. A nonempty slot has a
 * non-zero hash. We never clear hashes so this monotonically increases */
static uint64_t* hcounter = NULL;
/* The number of nonempty filled slots in the hashtable. A nonempty filled slot
 * has a non-zero hash AND a non-null addr. It increments when we write data
 * into a slot with addr==NULL and decrements when we clear data from a slot */
static uint64_t* hcounter_filled = NULL;

/* A counter increasing globally across all forks. */
static uintptr_t* counter = NULL;

/* Each process reserves a range of values at a time from the shared counter.
 * Should be a power of two for more efficient modulo calculation. */
#define COUNTER_RANGE 2048

/* Logging level for shared memory statistics
 * 0 = nothing
 * 1 = log totals, averages, min, max bytes marshalled and unmarshalled
 */
static size_t* log_level = NULL;

static double* sample_rate = NULL;

static size_t* compression = NULL;

static size_t* workers_should_exit = NULL;

static size_t* allow_removes = NULL;

/* Worker-local storage is cache line aligned. */
static char* locals;
#define LOCAL(id) ((local_t *)(locals + id * CACHE_ALIGN(sizeof(local_t))))

/* This should only be used before forking */
static uintptr_t early_counter = 0;

/* The top of the heap */
static char** heap = NULL;

/* Useful to add assertions */
static pid_t* master_pid = NULL;
static pid_t my_pid = 0;

static size_t num_workers;

/* This is a process-local value. The master process is 0, workers are numbered
 * starting at 1. This is an offset into the worker local values in the heap. */
static size_t worker_id;

static size_t allow_hashtable_writes_by_current_process = 1;
static size_t worker_can_exit = 1;

/* Where the heap started (bottom) */
static char* heap_init = NULL;
/* Where the heap will end (top) */
static char* heap_max = NULL;

static size_t* wasted_heap_size = NULL;

static size_t used_heap_size(void) {
  return *heap - heap_init;
}

static long removed_count = 0;

static ZSTD_CCtx* zstd_cctx = NULL;
static ZSTD_DCtx* zstd_dctx = NULL;

/* Expose so we can display diagnostics */
CAMLprim value hh_used_heap_size(void) {
  if (shm_use_sharded_hashtbl) {
    return shmffi_allocated_bytes();
  }
  return Val_long(used_heap_size());
}

/* Part of the heap not reachable from hashtable entries. Can be reclaimed with
 * hh_collect. */
CAMLprim value hh_wasted_heap_size(void) {
  // TODO(hverr): Support sharded hash tables
  assert(wasted_heap_size != NULL);
  return Val_long(*wasted_heap_size);
}

CAMLprim value hh_log_level(void) {
  return Val_long(*log_level);
}

CAMLprim value hh_sample_rate(void) {
  CAMLparam0();
  CAMLreturn(caml_copy_double(*sample_rate));
}

CAMLprim value hh_hash_used_slots(void) {
  // TODO(hverr): For some reason this returns a tuple.
  // Fix this when the migration is complete.
  CAMLparam0();
  CAMLlocal2(connector, num_entries);

  connector = caml_alloc_tuple(2);
  if (shm_use_sharded_hashtbl) {
    num_entries = shmffi_num_entries();
    Store_field(connector, 0, num_entries);
    Store_field(connector, 1, num_entries);
  } else {
    Store_field(connector, 0, Val_long(*hcounter_filled));
    Store_field(connector, 1, Val_long(*hcounter));
  }

  CAMLreturn(connector);
}

CAMLprim value hh_hash_slots(void) {
  CAMLparam0();
  if (shm_use_sharded_hashtbl) {
    // In the sharded hash table implementation, we dynamically resize
    // the tables. As such, this doesn't really make sense. Return the
    // number of entries for now.
    CAMLreturn(shmffi_num_entries());
  }

  CAMLreturn(Val_long(hashtbl_size));
}

#ifdef _WIN32

static struct timeval log_duration(const char *prefix, struct timeval start_t) {
   return start_t; // TODO
}

#else

static struct timeval log_duration(const char *prefix, struct timeval start_t) {
  struct timeval end_t = {0};
  gettimeofday(&end_t, NULL);
  time_t secs = end_t.tv_sec - start_t.tv_sec;
  suseconds_t usecs = end_t.tv_usec - start_t.tv_usec;
  double time_taken = secs + ((double)usecs / 1000000);
  fprintf(stderr, "%s took %.2lfs\n", prefix, time_taken);
  return end_t;
}

#endif

#ifdef _WIN32

static HANDLE memfd;

/**************************************************************************
 * We create an anonymous memory file, whose `handle` might be
 * inherited by subprocesses.
 *
 * This memory file is tagged "reserved" but not "committed". This
 * means that the memory space will be reserved in the virtual memory
 * table but the pages will not be bound to any physical memory
 * yet. Further calls to 'VirtualAlloc' will "commit" pages, meaning
 * they will be bound to physical memory.
 *
 * This is behavior that should reflect the 'MAP_NORESERVE' flag of
 * 'mmap' on Unix. But, on Unix, the "commit" is implicit.
 *
 * Committing the whole shared heap at once would require the same
 * amount of free space in memory (or in swap file).
 **************************************************************************/
static void memfd_init(const char *shm_dir, size_t shared_mem_size, uint64_t minimum_avail) {
  memfd = CreateFileMapping(
    INVALID_HANDLE_VALUE,
    NULL,
    PAGE_READWRITE | SEC_RESERVE,
    shared_mem_size >> 32, shared_mem_size & ((1ll << 32) - 1),
    NULL);
  if (memfd == NULL) {
    win32_maperr(GetLastError());
    uerror("CreateFileMapping", Nothing);
  }
  if (!SetHandleInformation(memfd, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
    win32_maperr(GetLastError());
    uerror("SetHandleInformation", Nothing);
  }
}

#else

static int memfd_shared_mem = -1;
static int memfd_shmffi = -1;

static void raise_failed_anonymous_memfd_init(void) {
  static const value *exn = NULL;
  if (!exn) exn = caml_named_value("failed_anonymous_memfd_init");
  caml_raise_constant(*exn);
}

static void raise_less_than_minimum_available(uint64_t avail) {
  value arg;
  static const value *exn = NULL;
  if (!exn) exn = caml_named_value("less_than_minimum_available");
  arg = Val_long(avail);
  caml_raise_with_arg(*exn, arg);
}

#include <sys/statvfs.h>
static void assert_avail_exceeds_minimum(const char *shm_dir, uint64_t minimum_avail) {
  struct statvfs stats;
  uint64_t avail;
  if (statvfs(shm_dir, &stats)) {
    uerror("statvfs", caml_copy_string(shm_dir));
  }
  avail = stats.f_bsize * stats.f_bavail;
  if (avail < minimum_avail) {
    raise_less_than_minimum_available(avail);
  }
}

static int memfd_create_helper(const char *name, const char *shm_dir, size_t shared_mem_size, uint64_t minimum_avail) {
  int memfd = -1;

  if (shm_dir == NULL) {
    // This means that we should try to use the anonymous-y system calls
#if defined(MEMFD_CREATE)
    memfd = memfd_create(name, 0);
#endif
#if defined(__APPLE__)
    if (memfd < 0) {
      char memname[255];
      snprintf(memname, sizeof(memname), "/%s.%d", name, getpid());
      // the ftruncate below will fail with errno EINVAL if you try to
      // ftruncate the same sharedmem fd more than once. We're seeing this in
      // some tests, which might imply that two flow processes with the same
      // pid are starting up. This shm_unlink should prevent that from
      // happening. Here's a stackoverflow about it
      // http://stackoverflow.com/questions/25502229/ftruncate-not-working-on-posix-shared-memory-in-mac-os-x
      shm_unlink(memname);
      memfd = shm_open(memname, O_CREAT | O_RDWR, 0666);
      if (memfd < 0) {
          uerror("shm_open", Nothing);
      }

      // shm_open sets FD_CLOEXEC automatically. This is undesirable, because
      // we want this fd to be open for other processes, so that they can
      // reconnect to the shared memory.
      int fcntl_flags = fcntl(memfd, F_GETFD);
      if (fcntl_flags == -1) {
        printf("Error with fcntl(memfd): %s\n", strerror(errno));
        uerror("fcntl", Nothing);
      }
      // Unset close-on-exec
      fcntl(memfd, F_SETFD, fcntl_flags & ~FD_CLOEXEC);
    }
#endif
    if (memfd < 0) {
      raise_failed_anonymous_memfd_init();
    }
  } else {
    if (minimum_avail > 0) {
      assert_avail_exceeds_minimum(shm_dir, minimum_avail);
    }
    if (memfd < 0) {
      char template[1024];
      if (!snprintf(template, 1024, "%s/%s-XXXXXX", shm_dir, name)) {
        uerror("snprintf", Nothing);
      };
      memfd = mkstemp(template);
      if (memfd < 0) {
        uerror("mkstemp", caml_copy_string(template));
      }
      unlink(template);
    }
  }
  if(ftruncate(memfd, shared_mem_size) == -1) {
    uerror("ftruncate", Nothing);
  }
  return memfd;
}

/**************************************************************************
 * The memdfd_init function creates a anonymous memory file that might
 * be inherited by `Daemon.spawned` processus (contrary to a simple
 * anonymous mmap).
 *
 * The preferred mechanism is memfd_create(2) (see the upper
 * description).  Then we try shm_open(2) (on Apple OS X). As a safe fallback,
 * we use `mkstemp/unlink`.
 *
 * mkstemp is preferred over shm_open on Linux as it allows to
 * choose another directory that `/dev/shm` on system where this
 * partition is too small (e.g. the Travis containers).
 *
 * The resulting file descriptor should be mmaped with the memfd_map
 * function (see below).
 ****************************************************************************/
static void memfd_init(const char *shm_dir, size_t shared_mem_size, uint64_t minimum_avail) {
  memfd_shared_mem = memfd_create_helper("fb_heap", shm_dir, shared_mem_size, minimum_avail);
  if (shm_use_sharded_hashtbl) {
    memfd_shmffi = memfd_create_helper("fb_sharded_hashtbl", shm_dir, SHARDED_HASHTBL_MEM_SIZE, 0);
  }
}

#endif


/*****************************************************************************/
/* Given a pointer to the shared memory address space, initializes all
 * the globals that live in shared memory.
 */
/*****************************************************************************/

#ifdef _WIN32

static char *memfd_map(HANDLE memfd, char *mem_addr, size_t shared_mem_size) {
  char *mem = NULL;
  mem = MapViewOfFileEx(
    memfd,
    FILE_MAP_ALL_ACCESS,
    0, 0, 0,
    (char *)mem_addr);
  if (mem != mem_addr) {
    win32_maperr(GetLastError());
    uerror("MapViewOfFileEx", Nothing);
  }
  return mem;
}

#else

static char *memfd_map(int memfd, char *mem_addr, size_t shared_mem_size) {
  char *mem = NULL;
  /* MAP_NORESERVE is because we want a lot more virtual memory than what
   * we are actually going to use.
   */
  int flags = MAP_SHARED | MAP_NORESERVE | MAP_FIXED;
  int prot  = PROT_READ  | PROT_WRITE;
  mem =
    (char*)mmap((void *)mem_addr, shared_mem_size, prot,
                flags, memfd, 0);
  if(mem == MAP_FAILED) {
    printf("Error initializing: %s\n", strerror(errno));
    exit(2);
  }
  return mem;
}

#endif

/****************************************************************************
 * The function memfd_reserve force allocation of (mem -> mem+sz) in
 * the shared heap. This is mandatory on Windows. This is optional on
 * Linux but it allows to have explicit "Out of memory" error
 * messages. Otherwise, the kernel might terminate the process with
 * `SIGBUS`.
 ****************************************************************************/


static void raise_out_of_shared_memory(void)
{
  static const value *exn = NULL;
  if (!exn) exn = caml_named_value("out_of_shared_memory");
  caml_raise_constant(*exn);
}

#ifdef _WIN32

/* Reserves memory. This is required on Windows */
static void win_reserve(char * mem, size_t sz) {
  if (!VirtualAlloc(mem, sz, MEM_COMMIT, PAGE_READWRITE)) {
    win32_maperr(GetLastError());
    raise_out_of_shared_memory();
  }
}

/* On Linux, memfd_reserve is only used to reserve memory that is mmap'd to the
 * memfd file. Memory outside of that mmap does not need to be reserved, so we
 * don't call memfd_reserve on things like the temporary mmap used by
 * hh_collect. Instead, they use win_reserve() */
static void memfd_reserve(int memfd, char * mem, size_t sz) {
  (void)memfd;
  win_reserve(mem, sz);
}

#elif defined(__APPLE__)

/* So OSX lacks fallocate, but in general you can do
 * fcntl(fd, F_PREALLOCATE, &store)
 * however it doesn't seem to work for a shm_open fd, so this function is
 * currently a no-op. This means that our OOM handling for OSX is a little
 * weaker than the other OS's */
static void memfd_reserve(int memfd, char * mem, size_t sz) {
  (void)memfd;
  (void)mem;
  (void)sz;
}

#else

static void memfd_reserve(int memfd, char *mem, size_t sz) {
  off_t offset = (off_t)(mem - shared_mem);
  int err;
  do {
    err = posix_fallocate(memfd, offset, sz);
  } while (err == EINTR);
  if (err) {
    raise_out_of_shared_memory();
  }
}

#endif

// DON'T WRITE TO THE SHARED MEMORY IN THIS FUNCTION!!!  This function just
// calculates where the memory is and sets local globals. The shared memory
// might not be ready for writing yet! If you want to initialize a bit of
// shared memory, check out init_shared_globals
static void define_globals(char * shared_mem_init) {
  size_t page_size = getpagesize();
  char *mem = shared_mem_init;

  // Beginning of the shared memory
  shared_mem = mem;

#ifdef MADV_DONTDUMP
    // We are unlikely to get much useful information out of the shared heap in
    // a core file. Moreover, it can be HUGE, and the extensive work done dumping
    // it once for each CPU can mean that the user will reboot their machine
    // before the much more useful stack gets dumped!
    madvise(shared_mem, shared_mem_size, MADV_DONTDUMP);
#endif

  /* BEGINNING OF THE SMALL OBJECTS PAGE
   * We keep all the small objects in this page.
   * They are on different cache lines because we modify them atomically.
   */

  /* The pointer to the top of the heap.
   * We will atomically increment *heap every time we want to allocate.
   */
  heap = (char**)mem;

  // The number of elements in the hashtable
  assert(CACHE_LINE_SIZE >= sizeof(uint64_t));
  hcounter = (uint64_t*)(mem + CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(uintptr_t));
  counter = (uintptr_t*)(mem + 2*CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(pid_t));
  master_pid = (pid_t*)(mem + 3*CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(size_t));
  log_level = (size_t*)(mem + 4*CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(double));
  sample_rate = (double*)(mem + 5*CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(size_t));
  compression = (size_t*)(mem + 6*CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(size_t));
  workers_should_exit = (size_t*)(mem + 7*CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(size_t));
  wasted_heap_size = (size_t*)(mem + 8*CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(size_t));
  allow_removes = (size_t*)(mem + 9*CACHE_LINE_SIZE);

  assert (CACHE_LINE_SIZE >= sizeof(size_t));
  hcounter_filled = (size_t*)(mem + 10*CACHE_LINE_SIZE);

  mem += page_size;
  // Just checking that the page is large enough.
  assert(page_size > 11*CACHE_LINE_SIZE + (int)sizeof(int));

  assert (CACHE_LINE_SIZE >= sizeof(local_t));
  locals = mem;
  mem += locals_size_b;

  /* END OF THE SMALL OBJECTS PAGE */

  /* Global storage initialization */
  global_storage = (value*)mem;
  mem += global_size_b;

  /* Hashtable */
  hashtbl = (helt_t*)mem;
  mem += hashtbl_size_b;

  /* Heap */
  heap_init = mem;
  heap_max = heap_init + heap_size;

#ifdef _WIN32
  /* Reserve all memory space except the "huge" `global_size_b`. This is
   * required for Windows but we don't do this for Linux since it lets us run
   * more processes in parallel without running out of memory immediately
   * (though we do risk it later on) */
  memfd_reserve(memfd_shared_mem, (char *)global_storage, sizeof(global_storage[0]));
  memfd_reserve(memfd_shared_mem, (char *)heap, heap_init - (char *)heap);
#endif

}

/* The total size of the shared memory.  Most of it is going to remain
 * virtual. */
static size_t get_shared_mem_size(void) {
  size_t page_size = getpagesize();
  return (global_size_b + hashtbl_size_b +
          heap_size + page_size + locals_size_b);
}

// Must be called AFTER init_shared_globals / define_globals
// once per process, during hh_shared_init / hh_connect
static void init_zstd_compression() {
  // if use ZSTD
  if (*compression) {
    /* The resources below (dictionaries, contexts) technically leak,
     * we don't free them as there is no proper API from workers.
     * However, they are in use till the end of the process live. */
    zstd_cctx = ZSTD_createCCtx();
    zstd_dctx = ZSTD_createDCtx();
    {
      ZSTD_CDict* zstd_cdict = ZSTD_createCDict(dictionary_data, dictionary_data_len, *compression);
      const size_t result = ZSTD_CCtx_refCDict(zstd_cctx, zstd_cdict);
      assert(!ZSTD_isError(result));
    }
    {
      ZSTD_DDict* zstd_ddict = ZSTD_createDDict(dictionary_data, dictionary_data_len);
      const size_t result = ZSTD_DCtx_refDDict(zstd_dctx, zstd_ddict);
      assert(!ZSTD_isError(result));
    }
  }
}

static void init_shared_globals(
  size_t config_log_level,
  double config_sample_rate,
  size_t config_compression
) {
  // Initial size is zero for global storage is zero
  global_storage[0] = 0;
  // Initialize the number of element in the table
  *hcounter = 0;
  *hcounter_filled = 0;
  // Ensure the global counter starts on a COUNTER_RANGE boundary
  *counter = ALIGN(early_counter + 1, COUNTER_RANGE);
  *log_level = config_log_level;
  *sample_rate = config_sample_rate;
  *compression = config_compression;
  *workers_should_exit = 0;
  *wasted_heap_size = 0;
  *allow_removes = 1;

  for (uint64_t i = 0; i <= num_workers; i++) {
    LOCAL(i)->counter = 0;
  }

  // Initialize top heap pointers
  *heap = heap_init;
}

static void set_sizes(
  uint64_t config_global_size,
  uint64_t config_heap_size,
  uint64_t config_hash_table_pow,
  uint64_t config_num_workers) {

  size_t page_size = getpagesize();

  global_size = config_global_size;
  global_size_b = sizeof(global_storage[0]) + config_global_size;
  heap_size = config_heap_size;
  hash_table_pow = config_hash_table_pow;

  hashtbl_size    = 1ul << config_hash_table_pow;
  hashtbl_size_b  = hashtbl_size * sizeof(hashtbl[0]);

  // We will allocate a cache line for the master process and each worker
  // process, then pad that out to the nearest page.
  num_workers = config_num_workers;
  locals_size_b = ALIGN((1 + num_workers) * CACHE_LINE_SIZE, page_size);

  shared_mem_size = get_shared_mem_size();
}

/*****************************************************************************/
/* Must be called by the master BEFORE forking the workers! */
/*****************************************************************************/
CAMLprim value hh_shared_init(
  value config_val,
  value shm_dir_val,
  value num_workers_val
) {
  CAMLparam3(config_val, shm_dir_val, num_workers_val);
  CAMLlocal4(
    config_global_size_val,
    config_heap_size_val,
    config_hash_table_pow_val,
    config_shm_use_sharded_hashtbl
  );
  CAMLlocal1(
    config_shm_cache_size
  );

  config_global_size_val = Field(config_val, 0);
  config_heap_size_val = Field(config_val, 1);
  config_hash_table_pow_val = Field(config_val, 2);
  config_shm_use_sharded_hashtbl = Field(config_val, 3);
  config_shm_cache_size = Field(config_val, 4);

  set_sizes(
    Long_val(config_global_size_val),
    Long_val(config_heap_size_val),
    Long_val(config_hash_table_pow_val),
    Long_val(num_workers_val)
  );
  shm_use_sharded_hashtbl = Bool_val(config_shm_use_sharded_hashtbl);
  shm_cache_size_b = Long_val(config_shm_cache_size);

  // None -> NULL
  // Some str -> String_val(str)
  const char *shm_dir = NULL;
  if (shm_dir_val != Val_int(0)) {
    shm_dir = String_val(Field(shm_dir_val, 0));
  }

  memfd_init(
    shm_dir,
    shared_mem_size,
    Long_val(Field(config_val, 6))
  );
  assert(memfd_shared_mem >= 0);
  char *shared_mem_init = memfd_map(memfd_shared_mem, SHARED_MEM_INIT, shared_mem_size);
  define_globals(shared_mem_init);

  if (shm_use_sharded_hashtbl) {
    assert(memfd_shmffi >= 0);
    assert(SHARED_MEM_INIT + shared_mem_size <= SHARDED_HASHTBL_MEM_ADDR);
    char *mem_addr = memfd_map(memfd_shmffi, SHARDED_HASHTBL_MEM_ADDR, SHARDED_HASHTBL_MEM_SIZE);
    shmffi_init(mem_addr, SHARDED_HASHTBL_MEM_SIZE, shm_cache_size_b);
  }

  // Keeping the pids around to make asserts.
#ifdef _WIN32
  *master_pid = 0;
  my_pid = *master_pid;
#else
  *master_pid = getpid();
  my_pid = *master_pid;
#endif

  init_shared_globals(
    Long_val(Field(config_val, 7)),
    Double_val(Field(config_val, 8)),
    Long_val(Field(config_val, 9))
  );
  init_zstd_compression();
  // Checking that we did the maths correctly.
  assert(*heap + heap_size == shared_mem + shared_mem_size);

#ifndef _WIN32
  // Uninstall ocaml's segfault handler. It's supposed to throw an exception on
  // stack overflow, but we don't actually handle that exception, so what
  // happens in practice is we terminate at toplevel with an unhandled exception
  // and a useless ocaml backtrace. A core dump is actually more useful. Sigh.
  struct sigaction sigact = { 0 };
  sigact.sa_handler = SIG_DFL;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = 0;
  sigaction(SIGSEGV, &sigact, NULL);
#endif

  CAMLreturn(hh_get_handle());
}

/* Must be called by every worker before any operation is performed */
value hh_connect(value connector, value worker_id_val) {
  CAMLparam2(connector, worker_id_val);
  memfd_shared_mem = Handle_val(Field(connector, 0));
  set_sizes(
    Long_val(Field(connector, 1)),
    Long_val(Field(connector, 2)),
    Long_val(Field(connector, 3)),
    Long_val(Field(connector, 4))
  );
  shm_use_sharded_hashtbl = Bool_val(Field(connector, 5));
  shm_cache_size_b = Long_val(Field(connector, 6));
  memfd_shmffi = Handle_val(Field(connector, 7));
  worker_id = Long_val(worker_id_val);
#ifdef _WIN32
  my_pid = 1; // Trick
#else
  my_pid = getpid();
#endif
  assert(memfd_shared_mem >= 0);
  char *shared_mem_init = memfd_map(memfd_shared_mem, SHARED_MEM_INIT, shared_mem_size);
  define_globals(shared_mem_init);
  init_zstd_compression();

  if (shm_use_sharded_hashtbl) {
    assert(memfd_shmffi >= 0);
    char *mem_addr = memfd_map(memfd_shmffi, SHARDED_HASHTBL_MEM_ADDR, SHARDED_HASHTBL_MEM_SIZE);
    shmffi_attach(mem_addr, SHARDED_HASHTBL_MEM_SIZE);
  }

  CAMLreturn(Val_unit);
}

/* Can only be called after init or after earlier connect. */
value hh_get_handle(void) {
  CAMLparam0();
  CAMLlocal1(
      connector
  );
  connector = caml_alloc_tuple(8);
  Store_field(connector, 0, Val_handle(memfd_shared_mem));
  Store_field(connector, 1, Val_long(global_size));
  Store_field(connector, 2, Val_long(heap_size));
  Store_field(connector, 3, Val_long(hash_table_pow));
  Store_field(connector, 4, Val_long(num_workers));
  Store_field(connector, 5, Val_bool(shm_use_sharded_hashtbl));
  Store_field(connector, 6, Val_long(shm_cache_size_b));
  Store_field(connector, 7, Val_handle(memfd_shmffi));

  CAMLreturn(connector);
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

CAMLprim value hh_counter_next(void) {
  CAMLparam0();
  CAMLlocal1(result);

  uintptr_t v = 0;
  if (counter) {
    v = LOCAL(worker_id)->counter;
    if (v % COUNTER_RANGE == 0) {
      v = __atomic_fetch_add(counter, COUNTER_RANGE, __ATOMIC_RELAXED);
    }
    ++v;
    LOCAL(worker_id)->counter = v;
  } else {
    v = ++early_counter;
  }

  result = Val_long(v % Max_long); // Wrap around.
  CAMLreturn(result);
}

/*****************************************************************************/
/* There are a bunch of operations that only the designated master thread is
 * allowed to do. This assert will fail if the current process is not the master
 * process
 */
/*****************************************************************************/
static void assert_master(void) {
  assert(my_pid == *master_pid);
}

static void assert_not_master(void) {
  assert(my_pid != *master_pid);
}

static void assert_allow_removes(void) {
  assert(*allow_removes);
}

static void assert_allow_hashtable_writes_by_current_process(void) {
  assert(allow_hashtable_writes_by_current_process);
}

CAMLprim value hh_assert_master(void) {
  CAMLparam0();
  assert_master();
  CAMLreturn(Val_unit);
}

/*****************************************************************************/

CAMLprim value hh_stop_workers(void) {
  CAMLparam0();
  assert_master();
  *workers_should_exit = 1;
  CAMLreturn(Val_unit);
}

CAMLprim value hh_resume_workers(void) {
  CAMLparam0();
  assert_master();
  *workers_should_exit = 0;
  CAMLreturn(Val_unit);
}

CAMLprim value hh_set_can_worker_stop(value val) {
  CAMLparam1(val);
  worker_can_exit = Bool_val(val);
  CAMLreturn(Val_unit);
}

CAMLprim value hh_set_allow_removes(value val) {
  CAMLparam1(val);
  *allow_removes = Bool_val(val);
  CAMLreturn(Val_unit);
}

CAMLprim value hh_set_allow_hashtable_writes_by_current_process(value val) {
  CAMLparam1(val);
  allow_hashtable_writes_by_current_process = Bool_val(val);
  CAMLreturn(Val_unit);
}

static void check_should_exit(void) {
  if (workers_should_exit == NULL) {
    caml_failwith(
      "`check_should_exit` failed: `workers_should_exit` was uninitialized. "
      "Did you forget to call one of `hh_connect` or `hh_shared_init` "
      "to initialize shared memory before accessing it?"
    );
  } else if (*workers_should_exit) {
    static const value *exn = NULL;
    if (!exn) exn = caml_named_value("worker_should_exit");
    caml_raise_constant(*exn);
  }
}

CAMLprim value hh_check_should_exit (void) {
  CAMLparam0();
  check_should_exit();
  CAMLreturn(Val_unit);
}

/*****************************************************************************/
/* Global storage */
/*****************************************************************************/

void hh_shared_store(value data) {
  CAMLparam1(data);
  size_t size = caml_string_length(data);

  assert_master();                               // only the master can store
  assert(global_storage[0] == 0);                // Is it clear?
  assert(size < global_size_b - sizeof(global_storage[0])); // Do we have enough space?

  global_storage[0] = size;
  memfd_reserve(memfd_shared_mem, (char *)&global_storage[1], size);
  memcpy(&global_storage[1], &Field(data, 0), size);

  CAMLreturn0;
}

/*****************************************************************************/
/* We are allocating ocaml values. The OCaml GC must know about them.
 * caml_alloc_string might trigger the GC, when that happens, the GC needs
 * to scan the stack to find the OCaml roots. The macros CAMLparam0 and
 * CAMLlocal1 register the roots.
 */
/*****************************************************************************/

CAMLprim value hh_shared_load(void) {
  CAMLparam0();
  CAMLlocal1(result);

  size_t size = global_storage[0];
  assert(size != 0);
  result = caml_alloc_string(size);
  memcpy(&Field(result, 0), &global_storage[1], size);

  CAMLreturn(result);
}

void hh_shared_clear(void) {
  assert_master();
  global_storage[0] = 0;
}

value hh_check_heap_overflow(void) {
  if (shm_use_sharded_hashtbl) {
    return Val_bool(0);
  }

  if (*heap >= shared_mem + shared_mem_size) {
    return Val_bool(1);
  }
  return Val_bool(0);
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

CAMLprim value hh_collect(void) {
  if (shm_use_sharded_hashtbl != 0) {
    return Val_unit;
  }

  // NOTE: explicitly do NOT call CAMLparam or any of the other functions/macros
  // defined in caml/memory.h .
  // This function takes a boolean and returns unit.
  // Those are both immediates in the OCaml runtime.
  assert_master();
  assert_allow_removes();

  // Step 1: Walk the hashtbl entries, which are the roots of our marking pass.

  for (size_t i = 0; i < hashtbl_size; i++) {
    // Skip empty slots
    if (hashtbl[i].addr == NULL) { continue; }

    // No workers should be writing at the moment. If a worker died in the
    // middle of a write, that is also very bad
    assert(hashtbl[i].addr != HASHTBL_WRITE_IN_PROGRESS);

    // The hashtbl addr will be wrong after we relocate the heap entry, but we
    // don't know where the heap entry will relocate to yet. We need to first
    // move the heap entry, then fix up the hashtbl addr.
    //
    // We accomplish this by storing the heap header in the now useless addr
    // field and storing a pointer to the addr field where the header used to
    // be. Then, after moving the heap entry, we can follow the pointer to
    // restore our original header and update the addr field to our relocated
    // address.
    //
    // This is all super unsafe and only works because we constrain the size of
    // an hh_header_t struct to the size of a pointer.

    // Location of the addr field (8 bytes) in the hashtable
    char **hashtbl_addr = (char **)&hashtbl[i].addr;

    // Location of the header (8 bytes) in the heap
    char *heap_addr = (char *)hashtbl[i].addr;

    // Swap
    hh_header_t header = *(hh_header_t *)heap_addr;
    *(hh_header_t *)hashtbl_addr = header;
    *(uintptr_t *)heap_addr = (uintptr_t)hashtbl_addr;
  }

  // Step 2: Walk the heap and relocate entries, updating the hashtbl to point
  // to relocated addresses.

  // Pointer to free space in the heap where moved values will move to.
  char *dest = heap_init;

  // Pointer that walks the heap from bottom to top.
  char *src = heap_init;

  size_t aligned_size;
  hh_header_t header;
  while (src < *heap) {
    if (*(uint64_t *)src & 1) {
      // If the lsb is set, this is a header. If it's a header, that means the
      // entry was not marked in the first pass and should be collected. Don't
      // move dest pointer, but advance src pointer to next heap entry.
      header = *(hh_header_t *)src;
      aligned_size = HEAP_ALIGN(Heap_entry_total_size(header));
    } else {
      // If the lsb is 0, this is a pointer to the addr field of the hashtable
      // element, which holds the header bytes. This entry is live.
      char *hashtbl_addr = *(char **)src;
      header = *(hh_header_t *)hashtbl_addr;
      aligned_size = HEAP_ALIGN(Heap_entry_total_size(header));

      // Fix the hashtbl addr field to point to our new location and restore the
      // heap header data temporarily stored in the addr field bits.
      *(uintptr_t *)hashtbl_addr = (uintptr_t)dest;
      *(hh_header_t *)src = header;

      // Move the entry as far to the left as possible.
      memmove(dest, src, aligned_size);
      dest += aligned_size;
    }

    src += aligned_size;
  }

  // TODO: Space between dest and *heap is unused, but will almost certainly
  // become used again soon. Currently we will never decommit, which may cause
  // issues when there is memory pressure.
  //
  // If the kernel supports it, we might consider using madvise(MADV_FREE),
  // which allows the kernel to reclaim the memory lazily under pressure, but
  // would not force page faults under healthy operation.

  *heap = dest;
  *wasted_heap_size = 0;

  return Val_unit;
}

CAMLprim value hh_malloc_trim(void) {
#ifdef MALLOC_TRIM
  malloc_trim(0);
#endif
  return Val_unit;
}

static void raise_heap_full(void) {
  static const value *exn = NULL;
  if (!exn) exn = caml_named_value("heap_full");
  caml_raise_constant(*exn);
}

/*****************************************************************************/
/* Allocates in the shared heap. The chunks are cache aligned. */
/*****************************************************************************/

static heap_entry_t* hh_alloc(hh_header_t header, /*out*/size_t *total_size) {
  // the size of this allocation needs to be kept in sync with wasted_heap_size
  // modification in hh_remove
  size_t slot_size = HEAP_ALIGN(Heap_entry_total_size(header));
  *total_size = slot_size;
  char *chunk = __sync_fetch_and_add(heap, (char*) slot_size);
  if (chunk + slot_size > heap_max) {
    raise_heap_full();
  }
  memfd_reserve(memfd_shared_mem, chunk, slot_size);
  ((heap_entry_t *)chunk)->header = header;
  return (heap_entry_t *)chunk;
}

/*****************************************************************************/
/* Serializes an ocaml value into an Ocaml raw heap_entry (bytes) */
/*****************************************************************************/
value hh_serialize_raw(value data) {
  CAMLparam1(data);
  CAMLlocal1(result);
  char* data_value = NULL;
  size_t size = 0;
  size_t uncompressed_size = 0;
  storage_kind kind = 0;
  if (shm_use_sharded_hashtbl != 0) {
    CAMLreturn(shmffi_serialize_raw(data));
  }

  // If the data is an Ocaml string it is more efficient to copy its contents
  // directly instead of serializing it.
  if (Is_block(data) && Tag_val(data) == String_tag) {
    size = caml_string_length(data);
    data_value = malloc(size);
    memcpy(data_value, String_val(data), size);
    kind = KIND_STRING;
  } else {
    intnat serialized_size;
    // We are responsible for freeing the memory allocated by this function
    // After copying data_value we need to make sure to free data_value
    caml_output_value_to_malloc(
      data, Val_int(0)/*flags*/, &data_value, &serialized_size);

    assert(serialized_size >= 0);
    size = (size_t) serialized_size;
    kind = KIND_SERIALIZED;
  }

  // We limit the size of elements we will allocate to our heap to ~2GB
  assert(size < 0x80000000);

  size_t max_compression_size = 0;
  char* compressed_data = NULL;
  size_t compressed_size = 0;

  if (*compression) {
    max_compression_size = ZSTD_compressBound(size);
    compressed_data = malloc(max_compression_size);

    compressed_size = ZSTD_compress2(zstd_cctx, compressed_data, max_compression_size, data_value, size);
  }
  else {
    max_compression_size = LZ4_compressBound(size);
    compressed_data = malloc(max_compression_size);
    compressed_size = LZ4_compress_default(
      data_value,
      compressed_data,
      size,
      max_compression_size);
  }

  if (compressed_size != 0 && compressed_size < size) {
    uncompressed_size = size;
    size = compressed_size;
  }

  // Both size and uncompressed_size will certainly fit in 31 bits, as the
  // original size fits per the assert above and we check that the compressed
  // size is less than the original size.
  hh_header_t header
    = size << 33
    | (uint64_t)kind << 32
    | uncompressed_size << 1
    | 1;

  size_t ocaml_size = Heap_entry_total_size(header);
  result = caml_alloc_string(ocaml_size);
  heap_entry_t *addr = (heap_entry_t *)Bytes_val(result);
  addr->header = header;
  memcpy(&addr->data,
         uncompressed_size ? compressed_data : data_value,
         size);

  free(compressed_data);
  // We temporarily allocate memory using malloc to serialize the Ocaml object.
  // When we have finished copying the serialized data we need to free the
  // memory we allocated to avoid a leak.
  free(data_value);

  CAMLreturn(result);
}

/*****************************************************************************/
/* Allocates an ocaml value in the shared heap.
 * Any ocaml value is valid, except closures. It returns the address of
 * the allocated chunk.
 */
/*****************************************************************************/
static heap_entry_t* hh_store_ocaml(
  value data,
  /*out*/size_t *alloc_size,
  /*out*/size_t *orig_size,
  /*out*/size_t *total_size
) {
  char* data_value = NULL;
  size_t size = 0;
  size_t uncompressed_size = 0;
  storage_kind kind = 0;

  // If the data is an Ocaml string it is more efficient to copy its contents
  // directly in our heap instead of serializing it.
  if (Is_block(data) && Tag_val(data) == String_tag) {
    size = caml_string_length(data);
    data_value = malloc(size);
    memcpy(data_value, String_val(data), size);
    kind = KIND_STRING;
  } else {
    intnat serialized_size;
    // We are responsible for freeing the memory allocated by this function
    // After copying data_value into our object heap we need to make sure to free
    // data_value
    caml_output_value_to_malloc(
      data, Val_int(0)/*flags*/, &data_value, &serialized_size);

    assert(serialized_size >= 0);
    size = (size_t) serialized_size;
    kind = KIND_SERIALIZED;
  }

  // We limit the size of elements we will allocate to our heap to ~2GB
  assert(size < 0x80000000);
  *orig_size = size;

  size_t max_compression_size = 0;
  char* compressed_data = NULL;
  size_t compressed_size = 0;

  if (*compression) {
    max_compression_size = ZSTD_compressBound(size);
    compressed_data = malloc(max_compression_size);

    compressed_size = ZSTD_compress2(zstd_cctx, compressed_data, max_compression_size, data_value, size);
  }
  else {
    max_compression_size = LZ4_compressBound(size);
    compressed_data = malloc(max_compression_size);
    compressed_size = LZ4_compress_default(
      data_value,
      compressed_data,
      size,
      max_compression_size);
  }

  if (compressed_size != 0 && compressed_size < size) {
    uncompressed_size = size;
    size = compressed_size;
  }

  *alloc_size = size;

  // Both size and uncompressed_size will certainly fit in 31 bits, as the
  // original size fits per the assert above and we check that the compressed
  // size is less than the original size.
  hh_header_t header
    = size << 33
    | (uint64_t)kind << 32
    | uncompressed_size << 1
    | 1;

  heap_entry_t* addr = hh_alloc(header, total_size);
  memcpy(&addr->data,
         uncompressed_size ? compressed_data : data_value,
         size);

  free(compressed_data);
  // We temporarily allocate memory using malloc to serialize the Ocaml object.
  // When we have finished copying the serialized data into our heap we need
  // to free the memory we allocated to avoid a leak.
  free(data_value);

  return addr;
}

/*****************************************************************************/
/* Given an OCaml string, returns the 8 first bytes in an unsigned long.
 * The key is generated using MD5, but we only use the first 8 bytes because
 * it allows us to use atomic operations.
 */
/*****************************************************************************/
static uint64_t get_hash(value key) {
  return *((uint64_t*)String_val(key));
}

CAMLprim value hh_get_hash_ocaml(value key) {
  return caml_copy_int64(*((uint64_t*)String_val(key)));
}

/*****************************************************************************/
/* Writes the data in one of the slots of the hashtable. There might be
 * concurrent writers, when that happens, the first writer wins.
 *
 * Returns a tuple (compressed_size, original_size, total_size) where...
 *   original_size ("orig_size") is the size in bytes of the marshalled value,
 *   compressed_size ("alloc_size") is byte size after that blob has been compressed by ZSTD or LZ4,
 *   total_size ("total_size") is byte size for that compressed blob, plus header, aligned.
 * If the slot was already written to, a negative value is returned for each element of the tuple.
 */
/*****************************************************************************/
static value write_at(unsigned int slot, value data) {
  CAMLparam1(data);
  CAMLlocal1(result);
  result = caml_alloc_tuple(3);
  // Try to write in a value to indicate that the data is being written.
  if(
     __sync_bool_compare_and_swap(
       &(hashtbl[slot].addr),
       NULL,
       HASHTBL_WRITE_IN_PROGRESS
     )
  ) {
    assert_allow_hashtable_writes_by_current_process();
    size_t alloc_size = 0;
    size_t orig_size = 0;
    size_t total_size = 0;
    hashtbl[slot].addr = hh_store_ocaml(data, &alloc_size, &orig_size, &total_size);
    Store_field(result, 0, Val_long(alloc_size));
    Store_field(result, 1, Val_long(orig_size));
    Store_field(result, 2, Val_long(total_size));
    __sync_fetch_and_add(hcounter_filled, 1);
  } else {
    Store_field(result, 0, Min_long);
    Store_field(result, 1, Min_long);
    Store_field(result, 2, Min_long);
  }
  CAMLreturn(result);
}

static void raise_hash_table_full(void) {
  static const value *exn = NULL;
  if (!exn) exn = caml_named_value("hash_table_full");
  caml_raise_constant(*exn);
}

/*****************************************************************************/
/* Adds a key value to the hashtable. This code is perf sensitive, please
 * check the perf before modifying.
 *
 * Returns a tuple (compressed_size, original_size, total_size) where
 *   original_size ("orig_size") is the size in bytes of the marshalled value,
 *   compressed_size ("alloc_size") is byte size after that blob has been compressed by ZSTD or LZ4,
 *   total_size ("total_size") is byte size for that compressed blob, plus header, aligned.
 * But if nothing new was added, then all three numbers returned are negative.
 */
/*****************************************************************************/
value hh_add(value evictable, value key, value data) {
  CAMLparam3(evictable, key, data);
  uint64_t hash = get_hash(key);
  if (shm_use_sharded_hashtbl != 0) {
    _Bool eviction_enabled = shm_cache_size_b >= 0;
    CAMLreturn(shmffi_add(Bool_val(evictable) && eviction_enabled, hash, data));
  }
  check_should_exit();
  unsigned int slot = hash & (hashtbl_size - 1);
  unsigned int init_slot = slot;
  while(1) {
    uint64_t slot_hash = hashtbl[slot].hash;

    if(slot_hash == hash) {
      // overwrite previous value for this hash
      CAMLreturn(write_at(slot, data));
    }

    if (*hcounter >= hashtbl_size) {
      // We're never going to find a spot
      raise_hash_table_full();
    }

    if(slot_hash == 0) {
      // We think we might have a free slot, try to atomically grab it.
      if(__sync_bool_compare_and_swap(&(hashtbl[slot].hash), 0, hash)) {
        uint64_t size = __sync_fetch_and_add(hcounter, 1);
        // Sanity check
        assert(size < hashtbl_size);
        CAMLreturn(write_at(slot, data));
      }

      // Grabbing it failed -- why? If someone else is trying to insert
      // the data we were about to, try to insert it ourselves too.
      // Otherwise, keep going.
      // Note that this read relies on the __sync call above preventing the
      // compiler from caching the value read out of memory. (And of course
      // isn't safe on any arch that requires memory barriers.)
      if(hashtbl[slot].hash == hash) {
        // Some other thread already grabbed this slot to write this
        // key, but they might not have written the address (or even
        // the sigil value) yet. We can't return from hh_add until we
        // know that hh_mem would succeed, which is to say that addr is
        // no longer null. To make sure hh_mem will work, we try
        // writing the value ourselves; either we insert it ourselves or
        // we know the address is now non-NULL.
        CAMLreturn(write_at(slot, data));
      }
    }

    slot = (slot + 1) & (hashtbl_size - 1);
    if (slot == init_slot) {
      // We're never going to find a spot
      raise_hash_table_full();
    }
  }
}

/*****************************************************************************/
/* Stores a raw bytes representation of an heap_entry in the shared heap.  It
 * returns the address of the allocated chunk.
 */
/*****************************************************************************/
static heap_entry_t* hh_store_raw_entry(
  value data
) {
  size_t size = caml_string_length(data) - sizeof(heap_entry_t);
  size_t total_size = 0;
  heap_entry_t* entry = (heap_entry_t*)Bytes_val(data);

  hh_header_t header = entry->header;
  heap_entry_t* addr = hh_alloc(header, &total_size);
  memcpy(&addr->data,
         entry->data,
         size);
  return addr;
}

/*****************************************************************************/
/* Writes the raw serialized data in one of the slots of the hashtable. There
 * might be concurrent writers, when that happens, the first writer wins.
 *
 */
/*****************************************************************************/
static value write_raw_at(unsigned int slot, value data) {
  CAMLparam1(data);
  // Try to write in a value to indicate that the data is being written.
  if(
     __sync_bool_compare_and_swap(
       &(hashtbl[slot].addr),
       NULL,
       HASHTBL_WRITE_IN_PROGRESS
     )
  ) {
    assert_allow_hashtable_writes_by_current_process();
    hashtbl[slot].addr = hh_store_raw_entry(data);
    __sync_fetch_and_add(hcounter_filled, 1);
  }
  CAMLreturn(Val_unit);
}

/*****************************************************************************/
/* Adds a key and raw heap_entry (represented as bytes) to the hashtable. Used
 * for over the network proxying.
 *
 * Returns unit.
 */
/*****************************************************************************/
CAMLprim value hh_add_raw(value key, value data) {
  CAMLparam2(key, data);
  uint64_t hash = get_hash(key);
  if (shm_use_sharded_hashtbl != 0) {
    CAMLreturn(shmffi_add_raw(hash, data));
  }
  check_should_exit();
  unsigned int slot = hash & (hashtbl_size - 1);
  unsigned int init_slot = slot;
  while(1) {
    uint64_t slot_hash = hashtbl[slot].hash;

    if(slot_hash == hash) {
      CAMLreturn(write_raw_at(slot, data));
    }

    if (*hcounter >= hashtbl_size) {
      // We're never going to find a spot
      raise_hash_table_full();
    }

    if(slot_hash == 0) {
      // We think we might have a free slot, try to atomically grab it.
      if(__sync_bool_compare_and_swap(&(hashtbl[slot].hash), 0, hash)) {
        uint64_t size = __sync_fetch_and_add(hcounter, 1);
        // Sanity check
        assert(size < hashtbl_size);
        CAMLreturn(write_raw_at(slot, data));
      }

      // Grabbing it failed -- why? If someone else is trying to insert
      // the data we were about to, try to insert it ourselves too.
      // Otherwise, keep going.
      // Note that this read relies on the __sync call above preventing the
      // compiler from caching the value read out of memory. (And of course
      // isn't safe on any arch that requires memory barriers.)
      if(hashtbl[slot].hash == hash) {
        // Some other thread already grabbed this slot to write this
        // key, but they might not have written the address (or even
        // the sigil value) yet. We can't return from hh_add until we
        // know that hh_mem would succeed, which is to say that addr is
        // no longer null. To make sure hh_mem will work, we try
        // writing the value ourselves; either we insert it ourselves or
        // we know the address is now non-NULL.
        CAMLreturn(write_raw_at(slot, data));
      }
    }

    slot = (slot + 1) & (hashtbl_size - 1);
    if (slot == init_slot) {
      // We're never going to find a spot
      raise_hash_table_full();
    }
  }
  CAMLreturn(Val_unit);
}

/*****************************************************************************/
/* Finds the slot corresponding to the key in a hash table. The returned slot
 * is either free or points to the key.
 */
/*****************************************************************************/
static unsigned int find_slot(value key) {
  uint64_t hash = get_hash(key);
  unsigned int slot = hash & (hashtbl_size - 1);
  unsigned int init_slot = slot;
  while(1) {
    if(hashtbl[slot].hash == hash) {
      return slot;
    }
    if(hashtbl[slot].hash == 0) {
      return slot;
    }
    slot = (slot + 1) & (hashtbl_size - 1);

    if (slot == init_slot) {
      raise_hash_table_full();
    }
  }
}

static _Bool hh_is_slot_taken_for_key(unsigned int slot, value key) {
  _Bool good_hash = hashtbl[slot].hash == get_hash(key);
  _Bool non_null_addr = hashtbl[slot].addr != NULL;
  if (good_hash && non_null_addr) {
    // The data is currently in the process of being written, wait until it
    // actually is ready to be used before returning.
    time_t start = 0;
    while (hashtbl[slot].addr == HASHTBL_WRITE_IN_PROGRESS) {
#if defined(__aarch64__)
      asm volatile("yield" : : : "memory");
#else
      asm volatile("pause" : : : "memory");
#endif
      // if the worker writing the data dies, we can get stuck. Timeout check
      // to prevent it.
      time_t now = time(0);
      if (start == 0 || start > now) {
        start = now;
      } else if (now - start > 60) {
        caml_failwith("hh_mem busy-wait loop stuck for 60s");
      }
    }
    return 1;
  }
  return 0;
}

_Bool hh_mem_inner(value key) {
  check_should_exit();
  unsigned int slot = find_slot(key);
  return hh_is_slot_taken_for_key(slot, key);
}

/*****************************************************************************/
/* Returns true if the key is present. We need to check both the hash and
 * the address of the data. This is due to the fact that we remove by setting
 * the address slot to NULL (we never remove a hash from the table, outside
 * of garbage collection).
 */
/*****************************************************************************/
value hh_mem(value key) {
  CAMLparam1(key);
  if (shm_use_sharded_hashtbl != 0) {
    CAMLreturn(shmffi_mem(get_hash(key)));
  }
  CAMLreturn(Val_bool(hh_mem_inner(key) == 1));
}

/*****************************************************************************/
/* Deserializes the value pointed to by elt. */
/*****************************************************************************/
static CAMLprim value hh_deserialize(heap_entry_t *elt) {
  CAMLparam0();
  CAMLlocal1(result);
  size_t size = Entry_size(elt->header);
  size_t uncompressed_size_exp = Entry_uncompressed_size(elt->header);
  char *src = elt->data;
  char *data = elt->data;
  if (uncompressed_size_exp) {
    data = malloc(uncompressed_size_exp);
  size_t uncompressed_size = 0;
  if (*compression) {
    uncompressed_size = ZSTD_decompressDCtx(zstd_dctx, data, uncompressed_size_exp, src, size);
  }
  else {
    uncompressed_size = LZ4_decompress_safe(
      src,
      data,
      size,
      uncompressed_size_exp);
  }
    assert(uncompressed_size == uncompressed_size_exp);
    size = uncompressed_size;
  }

  if (Entry_kind(elt->header) == KIND_STRING) {
    result = caml_alloc_initialized_string(size, data);
  } else {
    result = caml_input_value_from_block(data, size);
  }

  if (data != src) {
    free(data);
  }
  CAMLreturn(result);
}

/*****************************************************************************/
/* Returns the value associated to a given key, and deserialize it. */
/* Returns [None] if the slot for the key is empty. */
/*****************************************************************************/
CAMLprim value hh_get_and_deserialize(value key) {
  CAMLparam1(key);
  check_should_exit();
  CAMLlocal2(deserialized_value, result);
  if (shm_use_sharded_hashtbl != 0) {
    CAMLreturn(shmffi_get_and_deserialize(get_hash(key)));
  }

  unsigned int slot = find_slot(key);
  if (!hh_is_slot_taken_for_key(slot, key)) {
    CAMLreturn(Val_none);
  }
  deserialized_value = hh_deserialize(hashtbl[slot].addr);
  result = hh_shared_caml_alloc_some(deserialized_value);
  CAMLreturn(result);
}

/*****************************************************************************/
/* Returns Ocaml bytes representing the raw heap_entry. */
/* Returns [None] if the slot for the key is empty. */
/*****************************************************************************/
CAMLprim value hh_get_raw(value key) {
  CAMLparam1(key);
  if (shm_use_sharded_hashtbl != 0) {
    CAMLreturn(shmffi_get_raw(get_hash(key)));
  }
  check_should_exit();
  CAMLlocal2(result, bytes);

  unsigned int slot = find_slot(key);
  if (!hh_is_slot_taken_for_key(slot, key)) {
    CAMLreturn(Val_none);
  }

  heap_entry_t *elt = hashtbl[slot].addr;
  size_t size = Heap_entry_total_size(elt->header);
  char *data = (char *)elt;
  bytes = caml_alloc_string(size);
  memcpy(Bytes_val(bytes), data, size);
  result = hh_shared_caml_alloc_some(bytes);
  CAMLreturn(result);
}

/*****************************************************************************/
/* Returns result of deserializing and possibly uncompressing a raw heap_entry
 * passed in as Ocaml bytes. */
/*****************************************************************************/
CAMLprim value hh_deserialize_raw(value heap_entry) {
  CAMLparam1(heap_entry);
  CAMLlocal1(result);
  if (shm_use_sharded_hashtbl != 0) {
    CAMLreturn(shmffi_deserialize_raw(heap_entry));
  }

  heap_entry_t* entry = (heap_entry_t*)Bytes_val(heap_entry);
  result = hh_deserialize(entry);
  CAMLreturn(result);
}

/*****************************************************************************/
/* Returns the compressed_size of the value associated to a given key. */
/* The key MUST be present. */
/*****************************************************************************/
CAMLprim value hh_get_size(value key) {
  CAMLparam1(key);
  if (shm_use_sharded_hashtbl != 0) {
    CAMLreturn(shmffi_get_size(get_hash(key)));
  }

  unsigned int slot = find_slot(key);
  assert(hashtbl[slot].hash == get_hash(key));
  CAMLreturn(Val_long(Entry_size(hashtbl[slot].addr->header)));
}

/*****************************************************************************/
/* Moves the data associated to key1 to key2.
 * key1 must be present.
 * key2 must be free.
 * Only the master can perform this operation.
 */
/*****************************************************************************/
void hh_move(value key1, value key2) {
  if (shm_use_sharded_hashtbl != 0) {
    shmffi_move(get_hash(key1), get_hash(key2));
    return;
  }

  unsigned int slot1 = find_slot(key1);
  unsigned int slot2 = find_slot(key2);

  assert_master();
  assert_allow_removes();
  assert(hashtbl[slot1].hash == get_hash(key1));
  assert(hashtbl[slot2].addr == NULL);
  // We are taking up a previously empty slot. Let's increment the counter.
  // hcounter_filled doesn't change, since slot1 becomes empty and slot2 becomes
  // filled.
  if (hashtbl[slot2].hash == 0) {
    __sync_fetch_and_add(hcounter, 1);
  }
  hashtbl[slot2].hash = get_hash(key2);
  hashtbl[slot2].addr = hashtbl[slot1].addr;
  hashtbl[slot1].addr = NULL;
}

/*****************************************************************************/
/* Removes a key from the hash table, and returns the compressed_size that thing used to take.
 * Undefined behavior if the key doesn't exist.
 * Only the master can perform this operation.
 */
/*****************************************************************************/
CAMLprim value hh_remove(value key) {
  CAMLparam1(key);
  if (shm_use_sharded_hashtbl != 0) {
    CAMLreturn(shmffi_remove(get_hash(key)));
  }

  unsigned int slot = find_slot(key);

  assert_master();
  assert_allow_removes();
  assert(hashtbl[slot].hash == get_hash(key));
  size_t entry_size = Entry_size(hashtbl[slot].addr->header);
  // see hh_alloc for the source of this size
  size_t slot_size =
    HEAP_ALIGN(Heap_entry_total_size(hashtbl[slot].addr->header));
  __sync_fetch_and_add(wasted_heap_size, slot_size);
  hashtbl[slot].addr = NULL;
  removed_count += 1;
  __sync_fetch_and_sub(hcounter_filled, 1);
  CAMLreturn(Val_long(entry_size));
}

CAMLprim value hh_removed_count(value ml_unit) {
    // TODO(hverr): Support sharded hash tables
    CAMLparam1(ml_unit);
    UNUSED(ml_unit);
    return Val_long(removed_count);
}
