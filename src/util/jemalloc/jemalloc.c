/*-
 * Copyright (C) 2009 Facebook, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of Facebook, Inc. nor the names of its contributors may
 *   be used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 * Copyright (C) 2006-2008 Jason Evans <jasone@FreeBSD.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 * This allocator implementation is designed to provide scalable performance
 * for multi-threaded programs on multi-processor systems.  The following
 * features are included for this purpose:
 *
 *   + Multiple arenas are used if there are multiple CPUs, which reduces lock
 *     contention and cache sloshing.
 *
 *   + Thread-specific caching is used if there are multiple threads, which
 *     reduces the amount of locking.
 *
 *   + Cache line sharing between arenas is avoided for internal data
 *     structures.
 *
 *   + Memory is managed in chunks and runs (chunks can be split into runs),
 *     rather than as individual pages.  This provides a constant-time
 *     mechanism for associating allocations with particular arenas.
 *
 * Allocation requests are rounded up to the nearest size class, and no record
 * of the original request size is maintained.  Allocations are broken into
 * categories according to size class.  Assuming runtime defaults, 4 kB pages
 * and a 16 byte quantum on a 32-bit system, the size classes in each category
 * are as follows:
 *
 *   |=======================================|
 *   | Category | Subcategory      |    Size |
 *   |=======================================|
 *   | Small    | Tiny             |       2 |
 *   |          |                  |       4 |
 *   |          |                  |       8 |
 *   |          |------------------+---------|
 *   |          | Quantum-spaced   |      16 |
 *   |          |                  |      32 |
 *   |          |                  |      48 |
 *   |          |                  |     ... |
 *   |          |                  |      96 |
 *   |          |                  |     112 |
 *   |          |                  |     128 |
 *   |          |------------------+---------|
 *   |          | Cacheline-spaced |     192 |
 *   |          |                  |     256 |
 *   |          |                  |     320 |
 *   |          |                  |     384 |
 *   |          |                  |     448 |
 *   |          |                  |     512 |
 *   |          |------------------+---------|
 *   |          | Sub-page         |     760 |
 *   |          |                  |    1024 |
 *   |          |                  |    1280 |
 *   |          |                  |     ... |
 *   |          |                  |    3328 |
 *   |          |                  |    3584 |
 *   |          |                  |    3840 |
 *   |=======================================|
 *   | Large                       |    4 kB |
 *   |                             |    8 kB |
 *   |                             |   12 kB |
 *   |                             |     ... |
 *   |                             | 1012 kB |
 *   |                             | 1016 kB |
 *   |                             | 1020 kB |
 *   |=======================================|
 *   | Huge                        |    1 MB |
 *   |                             |    2 MB |
 *   |                             |    3 MB |
 *   |                             |     ... |
 *   |=======================================|
 *
 * A different mechanism is used for each category:
 *
 *   Small : Each size class is segregated into its own set of runs.  Each run
 *           maintains a bitmap of which regions are free/allocated.
 *
 *   Large : Each allocation is backed by a dedicated run.  Metadata are stored
 *           in the associated arena chunk header maps.
 *
 *   Huge : Each allocation is backed by a dedicated contiguous set of chunks.
 *          Metadata are stored in a separate red-black tree.
 *
 *******************************************************************************
 */

#include "jemalloc_defs.h"

#if 0
__FBSDID("$FreeBSD: src/lib/libc/stdlib/malloc.c,v 1.183 2008/12/01 10:20:59 jasone Exp $");
#endif

#include <sys/mman.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/uio.h>

#include <errno.h>
#include <limits.h>
#ifndef SIZE_T_MAX
#  define SIZE_T_MAX	SIZE_MAX
#endif
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#ifdef JEMALLOC_LAZY_LOCK
#include <dlfcn.h>
#endif

#ifndef __DECONST
#  define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif

#include "rb.h"

#ifdef JEMALLOC_DEBUG
   /* Disable inlining to make debugging easier. */
#  define inline
#endif

/* Size of stack-allocated buffer passed to strerror_r(). */
#define	STRERROR_BUF		64

/*
 * Minimum alignment of allocations is 2^QUANTUM_2POW bytes.
 */
#ifdef __i386__
#  define QUANTUM_2POW		4
#endif
#ifdef __ia64__
#  define QUANTUM_2POW		4
#endif
#ifdef __alpha__
#  define QUANTUM_2POW		4
#endif
#ifdef __sparc64__
#  define QUANTUM_2POW		4
#endif
#ifdef __amd64__
#  define QUANTUM_2POW		4
#endif
#ifdef __arm__
#  define QUANTUM_2POW		3
#endif
#ifdef __mips__
#  define QUANTUM_2POW		3
#endif
#ifdef __powerpc__
#  define QUANTUM_2POW		4
#endif

#define	QUANTUM			((size_t)(1U << QUANTUM_2POW))
#define	QUANTUM_MASK		(QUANTUM - 1)

#define	SIZEOF_PTR		(1U << SIZEOF_PTR_2POW)

/* sizeof(int) == (1U << SIZEOF_INT_2POW). */
#ifndef SIZEOF_INT_2POW
#  define SIZEOF_INT_2POW	2
#endif

/* We can't use TLS in non-PIC programs, since TLS relies on loader magic. */
#if (!defined(PIC) && !defined(NO_TLS))
#  define NO_TLS
#endif

#ifdef NO_TLS
   /* JEMALLOC_MAG requires TLS. */
#  ifdef JEMALLOC_MAG
#    undef JEMALLOC_MAG
#  endif
   /* JEMALLOC_BALANCE requires TLS. */
#  ifdef JEMALLOC_BALANCE
#    undef JEMALLOC_BALANCE
#  endif
#endif

/*
 * Size and alignment of memory chunks that are allocated by the OS's virtual
 * memory system.
 */
#define	CHUNK_2POW_DEFAULT	20

/* Maximum number of dirty pages per arena. */
#define	DIRTY_MAX_DEFAULT	(1U << 9)

/*
 * Maximum size of L1 cache line.  This is used to avoid cache line aliasing.
 * In addition, this controls the spacing of cacheline-spaced size classes.
 */
#define	CACHELINE_2POW		6
#define	CACHELINE		((size_t)(1U << CACHELINE_2POW))
#define	CACHELINE_MASK		(CACHELINE - 1)

/*
 * Subpages are an artificially designated partitioning of pages.  Their only
 * purpose is to support subpage-spaced size classes.
 *
 * There must be at least 4 subpages per page, due to the way size classes are
 * handled.
 */
#define	SUBPAGE_2POW		8
#define	SUBPAGE			((size_t)(1U << SUBPAGE_2POW))
#define	SUBPAGE_MASK		(SUBPAGE - 1)

#ifdef JEMALLOC_TINY
   /* Smallest size class to support. */
#  define TINY_MIN_2POW		1
#endif

/*
 * Maximum size class that is a multiple of the quantum, but not (necessarily)
 * a power of 2.  Above this size, allocations are rounded up to the nearest
 * power of 2.
 */
#define	QSPACE_MAX_2POW_DEFAULT	7

/*
 * Maximum size class that is a multiple of the cacheline, but not (necessarily)
 * a power of 2.  Above this size, allocations are rounded up to the nearest
 * power of 2.
 */
#define	CSPACE_MAX_2POW_DEFAULT	9

/*
 * RUN_MAX_OVRHD indicates maximum desired run header overhead.  Runs are sized
 * as small as possible such that this setting is still honored, without
 * violating other constraints.  The goal is to make runs as small as possible
 * without exceeding a per run external fragmentation threshold.
 *
 * We use binary fixed point math for overhead computations, where the binary
 * point is implicitly RUN_BFP bits to the left.
 *
 * Note that it is possible to set RUN_MAX_OVRHD low enough that it cannot be
 * honored for some/all object sizes, since there is one bit of header overhead
 * per object (plus a constant).  This constraint is relaxed (ignored) for runs
 * that are so small that the per-region overhead is greater than:
 *
 *   (RUN_MAX_OVRHD / (reg_size << (3+RUN_BFP))
 */
#define	RUN_BFP			12
/*                                    \/   Implicit binary fixed point. */
#define	RUN_MAX_OVRHD		0x0000003dU
#define	RUN_MAX_OVRHD_RELAX	0x00001800U

/* Put a cap on small object run size.  This overrides RUN_MAX_OVRHD. */
#define	RUN_MAX_SMALL	(12 * PAGE_SIZE)

/*
 * Adaptive spinning must eventually switch to blocking, in order to avoid the
 * potential for priority inversion deadlock.  Backing off past a certain point
 * can actually waste time.
 */
#define	SPIN_LIMIT_2POW		11

/*
 * Conversion from spinning to blocking is expensive; we use (1U <<
 * BLOCK_COST_2POW) to estimate how many more times costly blocking is than
 * worst-case spinning.
 */
#define	BLOCK_COST_2POW		4

#ifdef JEMALLOC_MAG
   /*
    * Default magazine size, in bytes.  max_rounds is calculated to make
    * optimal use of the space, leaving just enough room for the magazine
    * header.
    */
#  define MAG_SIZE_2POW_DEFAULT	9
#endif

#ifdef JEMALLOC_BALANCE
   /*
    * We use an exponential moving average to track recent lock contention,
    * where the size of the history window is N, and alpha=2/(N+1).
    *
    * Due to integer math rounding, very small values here can cause
    * substantial degradation in accuracy, thus making the moving average decay
    * faster than it would with precise calculation.
    */
#  define BALANCE_ALPHA_INV_2POW	9

   /*
    * Threshold value for the exponential moving contention average at which to
    * re-assign a thread.
    */
#  define BALANCE_THRESHOLD_DEFAULT	(1U << (SPIN_LIMIT_2POW-4))
#endif

/******************************************************************************/

typedef pthread_mutex_t malloc_mutex_t;
typedef pthread_mutex_t malloc_spinlock_t;

/* Set to true once the allocator has been initialized. */
static bool malloc_initialized = false;

/* Used to let the initializing thread recursively allocate. */
static pthread_t malloc_initializer = (unsigned long)0;

/* Used to avoid initialization races. */
static malloc_mutex_t init_lock = PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

/******************************************************************************/
/*
 * Statistics data structures.
 */

#ifdef JEMALLOC_STATS

typedef struct malloc_bin_stats_s malloc_bin_stats_t;
struct malloc_bin_stats_s {
	/*
	 * Number of allocation requests that corresponded to the size of this
	 * bin.
	 */
	uint64_t	nrequests;

#ifdef JEMALLOC_MAG
	/* Number of magazine reloads from this bin. */
	uint64_t	nmags;
#endif

	/* Total number of runs created for this bin's size class. */
	uint64_t	nruns;

	/*
	 * Total number of runs reused by extracting them from the runs tree for
	 * this bin's size class.
	 */
	uint64_t	reruns;

	/* High-water mark for this bin. */
	unsigned long	highruns;

	/* Current number of runs in this bin. */
	unsigned long	curruns;
};

typedef struct arena_stats_s arena_stats_t;
struct arena_stats_s {
	/* Number of bytes currently mapped. */
	size_t		mapped;

	/*
	 * Total number of purge sweeps, total number of madvise calls made,
	 * and total pages purged in order to keep dirty unused memory under
	 * control.
	 */
	uint64_t	npurge;
	uint64_t	nmadvise;
	uint64_t	purged;

	/* Per-size-category statistics. */
	size_t		allocated_small;
	uint64_t	nmalloc_small;
	uint64_t	ndalloc_small;

	size_t		allocated_large;
	uint64_t	nmalloc_large;
	uint64_t	ndalloc_large;

#ifdef JEMALLOC_BALANCE
	/* Number of times this arena reassigned a thread due to contention. */
	uint64_t	nbalance;
#endif
};

typedef struct chunk_stats_s chunk_stats_t;
struct chunk_stats_s {
	/* Number of chunks that were allocated. */
	uint64_t	nchunks;

	/* High-water mark for number of chunks allocated. */
	unsigned long	highchunks;

	/*
	 * Current number of chunks allocated.  This value isn't maintained for
	 * any other purpose, so keep track of it in order to be able to set
	 * highchunks.
	 */
	unsigned long	curchunks;
};

#endif /* #ifdef JEMALLOC_STATS */

/******************************************************************************/
/*
 * Extent data structures.
 */

/* Tree of extents. */
typedef struct extent_node_s extent_node_t;
struct extent_node_s {
#ifdef JEMALLOC_DSS
	/* Linkage for the size/address-ordered tree. */
	rb_node(extent_node_t) link_szad;
#endif

	/* Linkage for the address-ordered tree. */
	rb_node(extent_node_t) link_ad;

	/* Pointer to the extent that this tree node is responsible for. */
	void	*addr;

	/* Total region size. */
	size_t	size;
};
typedef rb_tree(extent_node_t) extent_tree_t;

/******************************************************************************/
/*
 * Arena data structures.
 */

typedef struct arena_s arena_t;
typedef struct arena_bin_s arena_bin_t;

/* Each element of the chunk map corresponds to one page within the chunk. */
typedef struct arena_chunk_map_s arena_chunk_map_t;
struct arena_chunk_map_s {
	/*
	 * Linkage for run trees.  There are two disjoint uses:
	 *
	 * 1) arena_t's runs_avail tree.
	 * 2) arena_run_t conceptually uses this linkage for in-use non-full
	 *    runs, rather than directly embedding linkage.
	 */
	rb_node(arena_chunk_map_t)	link;

	/*
	 * Run address (or size) and various flags are stored together.  The bit
	 * layout looks like (assuming 32-bit system):
	 *
	 *   ???????? ???????? ????---- ---kdzla
	 *
	 * ? : Unallocated: Run address for first/last pages, unset for internal
	 *                  pages.
	 *     Small: Run address.
	 *     Large: Run size for first page, unset for trailing pages.
	 * - : Unused.
	 * k : key?
	 * d : dirty?
	 * z : zeroed?
	 * l : large?
	 * a : allocated?
	 *
	 * Following are example bit patterns for the three types of runs.
	 *
	 * r : run address
	 * s : run size
	 * x : don't care
	 * - : 0
	 * [dzla] : bit set
	 *
	 *   Unallocated:
	 *     ssssssss ssssssss ssss---- --------
	 *     xxxxxxxx xxxxxxxx xxxx---- ----d---
	 *     ssssssss ssssssss ssss---- -----z--
	 *
	 *   Small:
	 *     rrrrrrrr rrrrrrrr rrrr---- -------a
	 *     rrrrrrrr rrrrrrrr rrrr---- -------a
	 *     rrrrrrrr rrrrrrrr rrrr---- -------a
	 *
	 *   Large:
	 *     ssssssss ssssssss ssss---- ------la
	 *     -------- -------- -------- ------la
	 *     -------- -------- -------- ------la
	 */
	size_t				bits;
#define	CHUNK_MAP_KEY		((size_t)0x10U)
#define	CHUNK_MAP_DIRTY		((size_t)0x08U)
#define	CHUNK_MAP_ZEROED	((size_t)0x04U)
#define	CHUNK_MAP_LARGE		((size_t)0x02U)
#define	CHUNK_MAP_ALLOCATED	((size_t)0x01U)
};
typedef rb_tree(arena_chunk_map_t) arena_avail_tree_t;
typedef rb_tree(arena_chunk_map_t) arena_run_tree_t;

/* Arena chunk header. */
typedef struct arena_chunk_s arena_chunk_t;
struct arena_chunk_s {
	/* Arena that owns the chunk. */
	arena_t		*arena;

	/* Linkage for the arena's chunks_dirty tree. */
	rb_node(arena_chunk_t) link_dirty;

	/* Number of dirty pages. */
	size_t		ndirty;

	/* Map of pages within chunk that keeps track of free/large/small. */
	arena_chunk_map_t map[1]; /* Dynamically sized. */
};
typedef rb_tree(arena_chunk_t) arena_chunk_tree_t;

typedef struct arena_run_s arena_run_t;
struct arena_run_s {
#ifdef JEMALLOC_DEBUG
	uint32_t	magic;
#  define ARENA_RUN_MAGIC 0x384adf93
#endif

	/* Bin this run is associated with. */
	arena_bin_t	*bin;

	/* Index of first element that might have a free region. */
	unsigned	regs_minelm;

	/* Number of free regions in run. */
	unsigned	nfree;

	/* Bitmask of in-use regions (0: in use, 1: free). */
	unsigned	regs_mask[1]; /* Dynamically sized. */
};

struct arena_bin_s {
	/*
	 * Current run being used to service allocations of this bin's size
	 * class.
	 */
	arena_run_t	*runcur;

	/*
	 * Tree of non-full runs.  This tree is used when looking for an
	 * existing run when runcur is no longer usable.  We choose the
	 * non-full run that is lowest in memory; this policy tends to keep
	 * objects packed well, and it can also help reduce the number of
	 * almost-empty chunks.
	 */
	arena_run_tree_t runs;

	/* Size of regions in a run for this bin's size class. */
	size_t		reg_size;

	/* Total size of a run for this bin's size class. */
	size_t		run_size;

	/* Total number of regions in a run for this bin's size class. */
	uint32_t	nregs;

	/* Number of elements in a run's regs_mask for this bin's size class. */
	uint32_t	regs_mask_nelms;

	/* Offset of first region in a run for this bin's size class. */
	uint32_t	reg0_offset;

#ifdef JEMALLOC_STATS
	/* Bin statistics. */
	malloc_bin_stats_t stats;
#endif
};

struct arena_s {
#ifdef JEMALLOC_DEBUG
	uint32_t		magic;
#  define ARENA_MAGIC 0x947d3d24
#endif

	/* All operations on this arena require that lock be locked. */
	pthread_mutex_t		lock;

#ifdef JEMALLOC_STATS
	arena_stats_t		stats;
#endif

	/* Tree of dirty-page-containing chunks this arena manages. */
	arena_chunk_tree_t	chunks_dirty;

	/*
	 * In order to avoid rapid chunk allocation/deallocation when an arena
	 * oscillates right on the cusp of needing a new chunk, cache the most
	 * recently freed chunk.  The spare is left in the arena's chunk trees
	 * until it is deleted.
	 *
	 * There is one spare chunk per arena, rather than one spare total, in
	 * order to avoid interactions between multiple threads that could make
	 * a single spare inadequate.
	 */
	arena_chunk_t		*spare;

	/*
	 * Current count of pages within unused runs that are potentially
	 * dirty, and for which madvise(... MADV_DONTNEED) has not been called.
	 * By tracking this, we can institute a limit on how much dirty unused
	 * memory is mapped for each arena.
	 */
	size_t			ndirty;

	/*
	 * Size/address-ordered tree of this arena's available runs.  This tree
	 * is used for first-best-fit run allocation.
	 */
	arena_avail_tree_t	runs_avail;

#ifdef JEMALLOC_BALANCE
	/*
	 * The arena load balancing machinery needs to keep track of how much
	 * lock contention there is.  This value is exponentially averaged.
	 */
	uint32_t		contention;
#endif

	/*
	 * bins is used to store rings of free regions of the following sizes,
	 * assuming a 16-byte quantum, 4kB page size, and default
	 * JEMALLOC_OPTIONS.
	 *
	 *   bins[i] | size |
	 *   --------+------+
	 *        0  |    2 |
	 *        1  |    4 |
	 *        2  |    8 |
	 *   --------+------+
	 *        3  |   16 |
	 *        4  |   32 |
	 *        5  |   48 |
	 *        6  |   64 |
	 *           :      :
	 *           :      :
	 *       33  |  496 |
	 *       34  |  512 |
	 *   --------+------+
	 *       35  | 1024 |
	 *       36  | 2048 |
	 *   --------+------+
	 */
	arena_bin_t		bins[1]; /* Dynamically sized. */
};

/******************************************************************************/
/*
 * Magazine data structures.
 */

#ifdef JEMALLOC_MAG
typedef struct mag_s mag_t;
struct mag_s {
	size_t		binind; /* Index of associated bin. */
	size_t		nrounds;
	void		*rounds[1]; /* Dynamically sized. */
};

/*
 * Magazines are lazily allocated, but once created, they remain until the
 * associated mag_rack is destroyed.
 */
typedef struct bin_mags_s bin_mags_t;
struct bin_mags_s {
	mag_t	*curmag;
	mag_t	*sparemag;
};

typedef struct mag_rack_s mag_rack_t;
struct mag_rack_s {
	bin_mags_t	bin_mags[1]; /* Dynamically sized. */
};
#endif

/******************************************************************************/
/*
 * Data.
 */

#ifdef JEMALLOC_LAZY_LOCK
static bool isthreaded = false;
#else
#  define isthreaded true
#endif

/* Number of CPUs. */
static unsigned		ncpus;

/*
 * Page size.  STATIC_PAGE_SHIFT is determined by the configure script.  If
 * DYNAMIC_PAGE_SHIFT is enabled, only use the STATIC_PAGE_* macros where
 * compile-time values are required for the purposes of defining data
 * structures.
 */
#define	STATIC_PAGE_SIZE ((size_t)(1U << STATIC_PAGE_SHIFT))
#define	STATIC_PAGE_MASK ((size_t)(STATIC_PAGE_SIZE - 1))

#ifdef DYNAMIC_PAGE_SHIFT
static size_t		pagesize;
static size_t		pagesize_mask;
static size_t		pagesize_2pow;
#  define PAGE_SHIFT	pagesize_2pow
#  define PAGE_SIZE	pagesize
#  define PAGE_MASK	pagesize_mask
#else
#  define PAGE_SHIFT	STATIC_PAGE_SHIFT
#  define PAGE_SIZE	STATIC_PAGE_SIZE
#  define PAGE_MASK	STATIC_PAGE_MASK
#endif

/* Various bin-related settings. */
#ifdef JEMALLOC_TINY		/* Number of (2^n)-spaced tiny bins. */
#  define		ntbins	((unsigned)(QUANTUM_2POW - TINY_MIN_2POW))
#else
#  define		ntbins	0
#endif
static unsigned		nqbins; /* Number of quantum-spaced bins. */
static unsigned		ncbins; /* Number of cacheline-spaced bins. */
static unsigned		nsbins; /* Number of subpage-spaced bins. */
static unsigned		nbins;
#ifdef JEMALLOC_TINY
#  define		tspace_max	((size_t)(QUANTUM >> 1))
#endif
#define			qspace_min	QUANTUM
static size_t		qspace_max;
static size_t		cspace_min;
static size_t		cspace_max;
static size_t		sspace_min;
static size_t		sspace_max;
#define			bin_maxclass	sspace_max

static uint8_t const	*size2bin;
/*
 * const_size2bin is a static constant lookup table that in the common case can
 * be used as-is for size2bin.  For dynamically linked programs, this avoids
 * a page of memory overhead per process.
 */
#define	S2B_1(i)	i,
#define	S2B_2(i)	S2B_1(i) S2B_1(i)
#define	S2B_4(i)	S2B_2(i) S2B_2(i)
#define	S2B_8(i)	S2B_4(i) S2B_4(i)
#define	S2B_16(i)	S2B_8(i) S2B_8(i)
#define	S2B_32(i)	S2B_16(i) S2B_16(i)
#define	S2B_64(i)	S2B_32(i) S2B_32(i)
#define	S2B_128(i)	S2B_64(i) S2B_64(i)
#define	S2B_256(i)	S2B_128(i) S2B_128(i)
static const uint8_t	const_size2bin[STATIC_PAGE_SIZE - 255] = {
	S2B_1(0xffU)		/*    0 */
#if (QUANTUM_2POW == 4)
/* 64-bit system ************************/
#  ifdef JEMALLOC_TINY
	S2B_2(0)		/*    2 */
	S2B_2(1)		/*    4 */
	S2B_4(2)		/*    8 */
	S2B_8(3)		/*   16 */
#    define S2B_QMIN 3
#  else
	S2B_16(0)		/*   16 */
#    define S2B_QMIN 0
#  endif
	S2B_16(S2B_QMIN + 1)	/*   32 */
	S2B_16(S2B_QMIN + 2)	/*   48 */
	S2B_16(S2B_QMIN + 3)	/*   64 */
	S2B_16(S2B_QMIN + 4)	/*   80 */
	S2B_16(S2B_QMIN + 5)	/*   96 */
	S2B_16(S2B_QMIN + 6)	/*  112 */
	S2B_16(S2B_QMIN + 7)	/*  128 */
#  define S2B_CMIN (S2B_QMIN + 8)
#else
/* 32-bit system ************************/
#  ifdef JEMALLOC_TINY
	S2B_2(0)		/*    2 */
	S2B_2(1)		/*    4 */
	S2B_4(2)		/*    8 */
#    define S2B_QMIN 2
#  else
	S2B_8(0)		/*    8 */
#    define S2B_QMIN 0
#  endif
	S2B_8(S2B_QMIN + 1)	/*   16 */
	S2B_8(S2B_QMIN + 2)	/*   24 */
	S2B_8(S2B_QMIN + 3)	/*   32 */
	S2B_8(S2B_QMIN + 4)	/*   40 */
	S2B_8(S2B_QMIN + 5)	/*   48 */
	S2B_8(S2B_QMIN + 6)	/*   56 */
	S2B_8(S2B_QMIN + 7)	/*   64 */
	S2B_8(S2B_QMIN + 8)	/*   72 */
	S2B_8(S2B_QMIN + 9)	/*   80 */
	S2B_8(S2B_QMIN + 10)	/*   88 */
	S2B_8(S2B_QMIN + 11)	/*   96 */
	S2B_8(S2B_QMIN + 12)	/*  104 */
	S2B_8(S2B_QMIN + 13)	/*  112 */
	S2B_8(S2B_QMIN + 14)	/*  120 */
	S2B_8(S2B_QMIN + 15)	/*  128 */
#  define S2B_CMIN (S2B_QMIN + 16)
#endif
/****************************************/
	S2B_64(S2B_CMIN + 0)	/*  192 */
	S2B_64(S2B_CMIN + 1)	/*  256 */
	S2B_64(S2B_CMIN + 2)	/*  320 */
	S2B_64(S2B_CMIN + 3)	/*  384 */
	S2B_64(S2B_CMIN + 4)	/*  448 */
	S2B_64(S2B_CMIN + 5)	/*  512 */
#  define S2B_SMIN (S2B_CMIN + 6)
	S2B_256(S2B_SMIN + 0)	/*  768 */
	S2B_256(S2B_SMIN + 1)	/* 1024 */
	S2B_256(S2B_SMIN + 2)	/* 1280 */
	S2B_256(S2B_SMIN + 3)	/* 1536 */
	S2B_256(S2B_SMIN + 4)	/* 1792 */
	S2B_256(S2B_SMIN + 5)	/* 2048 */
	S2B_256(S2B_SMIN + 6)	/* 2304 */
	S2B_256(S2B_SMIN + 7)	/* 2560 */
	S2B_256(S2B_SMIN + 8)	/* 2816 */
	S2B_256(S2B_SMIN + 9)	/* 3072 */
	S2B_256(S2B_SMIN + 10)	/* 3328 */
	S2B_256(S2B_SMIN + 11)	/* 3584 */
	S2B_256(S2B_SMIN + 12)	/* 3840 */
#if (STATIC_PAGE_SHIFT == 13)
	S2B_256(S2B_SMIN + 13)	/* 4096 */
	S2B_256(S2B_SMIN + 14)	/* 4352 */
	S2B_256(S2B_SMIN + 15)	/* 4608 */
	S2B_256(S2B_SMIN + 16)	/* 4864 */
	S2B_256(S2B_SMIN + 17)	/* 5120 */
	S2B_256(S2B_SMIN + 18)	/* 5376 */
	S2B_256(S2B_SMIN + 19)	/* 5632 */
	S2B_256(S2B_SMIN + 20)	/* 5888 */
	S2B_256(S2B_SMIN + 21)	/* 6144 */
	S2B_256(S2B_SMIN + 22)	/* 6400 */
	S2B_256(S2B_SMIN + 23)	/* 6656 */
	S2B_256(S2B_SMIN + 24)	/* 6912 */
	S2B_256(S2B_SMIN + 25)	/* 7168 */
	S2B_256(S2B_SMIN + 26)	/* 7424 */
	S2B_256(S2B_SMIN + 27)	/* 7680 */
	S2B_256(S2B_SMIN + 28)	/* 7936 */
#endif
};
#undef S2B_1
#undef S2B_2
#undef S2B_4
#undef S2B_8
#undef S2B_16
#undef S2B_32
#undef S2B_64
#undef S2B_128
#undef S2B_256
#undef S2B_QMIN
#undef S2B_CMIN
#undef S2B_SMIN

#ifdef JEMALLOC_MAG
static size_t		max_rounds;
#endif

/* Various chunk-related settings. */
static size_t		chunksize;
static size_t		chunksize_mask; /* (chunksize - 1). */
static size_t		chunk_npages;
static size_t		arena_chunk_header_npages;
static size_t		arena_maxclass; /* Max size class for arenas. */

/********/
/*
 * Chunks.
 */

/* Protects chunk-related data structures. */
static malloc_mutex_t	huge_mtx;

/* Tree of chunks that are stand-alone huge allocations. */
static extent_tree_t	huge;

#ifdef JEMALLOC_DSS
/*
 * Protects sbrk() calls.  This avoids malloc races among threads, though it
 * does not protect against races with threads that call sbrk() directly.
 */
static malloc_mutex_t	dss_mtx;
/* Base address of the DSS. */
static void		*dss_base;
/* Current end of the DSS, or ((void *)-1) if the DSS is exhausted. */
static void		*dss_prev;
/* Current upper limit on DSS addresses. */
static void		*dss_max;

/*
 * Trees of chunks that were previously allocated (trees differ only in node
 * ordering).  These are used when allocating chunks, in an attempt to re-use
 * address space.  Depending on function, different tree orderings are needed,
 * which is why there are two trees with the same contents.
 */
static extent_tree_t	dss_chunks_szad;
static extent_tree_t	dss_chunks_ad;
#endif

#ifdef JEMALLOC_STATS
/* Huge allocation statistics. */
static uint64_t		huge_nmalloc;
static uint64_t		huge_ndalloc;
static size_t		huge_allocated;
#endif

/****************************/
/*
 * base (internal allocation).
 */

/*
 * Current pages that are being used for internal memory allocations.  These
 * pages are carved up in cacheline-size quanta, so that there is no chance of
 * false cache line sharing.
 */
static void		*base_pages;
static void		*base_next_addr;
static void		*base_past_addr; /* Addr immediately past base_pages. */
static extent_node_t	*base_nodes;
static malloc_mutex_t	base_mtx;
#ifdef JEMALLOC_STATS
static size_t		base_mapped;
#endif

/********/
/*
 * Arenas.
 */

/*
 * Arenas that are used to service external requests.  Not all elements of the
 * arenas array are necessarily used; arenas are created lazily as needed.
 */
static arena_t		**arenas;
static unsigned		narenas;
#ifndef NO_TLS
#  ifdef JEMALLOC_BALANCE
static unsigned		narenas_2pow;
#  else
static unsigned		next_arena;
#  endif
#endif
static pthread_mutex_t	arenas_lock; /* Protects arenas initialization. */

#ifndef NO_TLS
/*
 * Map of pthread_self() --> arenas[???], used for selecting an arena to use
 * for allocations.
 */
static __thread arena_t	*arenas_map;
#endif

#ifdef JEMALLOC_MAG
/*
 * Map of thread-specific magazine racks, used for thread-specific object
 * caching.
 */
static __thread mag_rack_t	*mag_rack;

/*
 * Same contents as mag_rack, but initialized such that the TSD destructor is
 * called when a thread exits, so that the cache can be cleaned up.
 */
static pthread_key_t		mag_rack_tsd;
#endif

#ifdef JEMALLOC_STATS
/* Chunk statistics. */
static chunk_stats_t	stats_chunks;
#endif

/*******************************/
/*
 * Runtime configuration options.
 */
const char	*jemalloc_options;

#ifdef JEMALLOC_DEBUG
static bool	opt_abort = true;
#  ifdef JEMALLOC_FILL
static bool	opt_junk = true;
#  endif
#else
static bool	opt_abort = false;
#  ifdef JEMALLOC_FILL
static bool	opt_junk = false;
#  endif
#endif
#ifdef JEMALLOC_DSS
static bool	opt_dss = true;
static bool	opt_mmap = true;
#endif
#ifdef JEMALLOC_MAG
static bool	opt_mag = true;
static size_t	opt_mag_size_2pow = MAG_SIZE_2POW_DEFAULT;
#endif
static size_t	opt_dirty_max = DIRTY_MAX_DEFAULT;
#ifdef JEMALLOC_BALANCE
static uint64_t	opt_balance_threshold = BALANCE_THRESHOLD_DEFAULT;
#endif
static bool	opt_print_stats = false;
static size_t	opt_qspace_max_2pow = QSPACE_MAX_2POW_DEFAULT;
static size_t	opt_cspace_max_2pow = CSPACE_MAX_2POW_DEFAULT;
static size_t	opt_chunk_2pow = CHUNK_2POW_DEFAULT;
#ifdef JEMALLOC_STATS
static bool	opt_utrace = false;
#endif
#ifdef JEMALLOC_SYSV
static bool	opt_sysv = false;
#endif
static bool	opt_xmalloc = false;
#ifdef JEMALLOC_FILL
static bool	opt_zero = false;
#endif
static int	opt_narenas_lshift = 0;

#ifdef JEMALLOC_STATS
typedef struct {
	void	*p;
	size_t	s;
	void	*r;
} malloc_utrace_t;

#define	UTRACE(a, b, c)							\
	if (opt_utrace) {						\
		malloc_utrace_t ut;					\
		ut.p = (a);						\
		ut.s = (b);						\
		ut.r = (c);						\
		utrace(&ut, sizeof(ut));				\
	}
#else
#define	UTRACE(a, b, c)
#endif

/******************************************************************************/
/*
 * Begin function prototypes for non-inline static functions.
 */

static bool	malloc_mutex_init(malloc_mutex_t *mutex);
static bool	malloc_spin_init(pthread_mutex_t *lock);
static void	wrtmessage(const char *p1, const char *p2, const char *p3,
		const char *p4);
#ifdef JEMALLOC_STATS
static void	malloc_printf(const char *format, ...);
#endif
static char	*umax2s(uintmax_t x, char *s);
#ifdef JEMALLOC_DSS
static bool	base_pages_alloc_dss(size_t minsize);
#endif
static bool	base_pages_alloc_mmap(size_t minsize);
static bool	base_pages_alloc(size_t minsize);
static void	*base_alloc(size_t size);
static void	*base_calloc(size_t number, size_t size);
static extent_node_t *base_node_alloc(void);
static void	base_node_dealloc(extent_node_t *node);
#ifdef JEMALLOC_STATS
static void	stats_print(arena_t *arena);
#endif
static void	*pages_map(void *addr, size_t size);
static void	pages_unmap(void *addr, size_t size);
#ifdef JEMALLOC_DSS
static void	*chunk_alloc_dss(size_t size);
static void	*chunk_recycle_dss(size_t size, bool zero);
#endif
static void	*chunk_alloc_mmap(size_t size);
static void	*chunk_alloc(size_t size, bool zero);
#ifdef JEMALLOC_DSS
static extent_node_t *chunk_dealloc_dss_record(void *chunk, size_t size);
static bool	chunk_dealloc_dss(void *chunk, size_t size);
#endif
static void	chunk_dealloc_mmap(void *chunk, size_t size);
static void	chunk_dealloc(void *chunk, size_t size);
#ifndef NO_TLS
static arena_t	*choose_arena_hard(void);
#endif
static void	arena_run_split(arena_t *arena, arena_run_t *run, size_t size,
    bool large, bool zero);
static arena_chunk_t *arena_chunk_alloc(arena_t *arena);
static void	arena_chunk_dealloc(arena_t *arena, arena_chunk_t *chunk);
static arena_run_t *arena_run_alloc(arena_t *arena, size_t size, bool large,
    bool zero);
static void	arena_purge(arena_t *arena);
static void	arena_run_dalloc(arena_t *arena, arena_run_t *run, bool dirty);
static void	arena_run_trim_head(arena_t *arena, arena_chunk_t *chunk,
    arena_run_t *run, size_t oldsize, size_t newsize);
static void	arena_run_trim_tail(arena_t *arena, arena_chunk_t *chunk,
    arena_run_t *run, size_t oldsize, size_t newsize, bool dirty);
static arena_run_t *arena_bin_nonfull_run_get(arena_t *arena, arena_bin_t *bin);
static void	*arena_bin_malloc_hard(arena_t *arena, arena_bin_t *bin);
static size_t	arena_bin_run_size_calc(arena_bin_t *bin, size_t min_run_size);
#ifdef JEMALLOC_BALANCE
static void	arena_lock_balance_hard(arena_t *arena);
#endif
#ifdef JEMALLOC_MAG
static void	mag_load(mag_t *mag);
#endif
static void	*arena_malloc_large(arena_t *arena, size_t size, bool zero);
static void	*arena_palloc(arena_t *arena, size_t alignment, size_t size,
    size_t alloc_size);
static size_t	arena_salloc(const void *ptr);
#ifdef JEMALLOC_MAG
static void	mag_unload(mag_t *mag);
#endif
static void	arena_dalloc_large(arena_t *arena, arena_chunk_t *chunk,
    void *ptr);
static void	arena_ralloc_large_shrink(arena_t *arena, arena_chunk_t *chunk,
    void *ptr, size_t size, size_t oldsize);
static bool	arena_ralloc_large_grow(arena_t *arena, arena_chunk_t *chunk,
    void *ptr, size_t size, size_t oldsize);
static bool	arena_ralloc_large(void *ptr, size_t size, size_t oldsize);
static void	*arena_ralloc(void *ptr, size_t size, size_t oldsize);
static bool	arena_new(arena_t *arena);
static arena_t	*arenas_extend(unsigned ind);
#ifdef JEMALLOC_MAG
static mag_t	*mag_create(arena_t *arena, size_t binind);
static void	mag_destroy(mag_t *mag);
static mag_rack_t *mag_rack_create(arena_t *arena);
static void	mag_rack_destroy(mag_rack_t *rack);
#endif
static void	*huge_malloc(size_t size, bool zero);
static void	*huge_palloc(size_t alignment, size_t size);
static void	*huge_ralloc(void *ptr, size_t size, size_t oldsize);
static void	huge_dalloc(void *ptr);
static void	malloc_print_stats(void);
#ifdef JEMALLOC_DEBUG
static void	size2bin_validate(void);
#endif
static bool	size2bin_init(void);
static bool	size2bin_init_hard(void);
static unsigned	malloc_ncpus(void);
static bool	malloc_init_hard(void);
static void	thread_cleanup(void *arg);
static void	jemalloc_prefork(void);
static void	jemalloc_postfork(void);

/*
 * End function prototypes.
 */
/******************************************************************************/

static void
wrtmessage(const char *p1, const char *p2, const char *p3, const char *p4)
{

	if (write(STDERR_FILENO, p1, strlen(p1)) < 0
	    || write(STDERR_FILENO, p2, strlen(p2)) < 0
	    || write(STDERR_FILENO, p3, strlen(p3)) < 0
	    || write(STDERR_FILENO, p4, strlen(p4)) < 0)
		return;
}

void	(*jemalloc_message)(const char *p1, const char *p2, const char *p3,
	    const char *p4) = wrtmessage;

/*
 * We don't want to depend on vsnprintf() for production builds, since that can
 * cause unnecessary bloat for static binaries.  umax2s() provides minimal
 * integer printing functionality, so that malloc_printf() use can be limited to
 * JEMALLOC_STATS code.
 */
#define	UMAX2S_BUFSIZE	21
static char *
umax2s(uintmax_t x, char *s)
{
	unsigned i;

	i = UMAX2S_BUFSIZE - 1;
	s[i] = '\0';
	do {
		i--;
		s[i] = "0123456789"[x % 10];
		x /= 10;
	} while (x > 0);

	return (&s[i]);
}

/*
 * Define a custom assert() in order to reduce the chances of deadlock during
 * assertion failure.
 */
#ifdef JEMALLOC_DEBUG
#  define assert(e) do {						\
	if (!(e)) {							\
		char line_buf[UMAX2S_BUFSIZE];				\
		jemalloc_message("<jemalloc>: ", __FILE__, ":",		\
		    umax2s(__LINE__, line_buf));			\
		jemalloc_message(": Failed assertion: ", "\"", #e,	\
		    "\"\n");						\
		abort();						\
	}								\
} while (0)
#else
#define assert(e)
#endif

#ifdef JEMALLOC_STATS
static int
utrace(const void *addr, size_t len)
{
	malloc_utrace_t *ut = (malloc_utrace_t *)addr;

	assert(len == sizeof(malloc_utrace_t));

	if (ut->p == NULL && ut->s == 0 && ut->r == NULL)
		malloc_printf("<jemalloc>:utrace: %d malloc_init()\n",
		    getpid());
	else if (ut->p == NULL && ut->r != NULL) {
		malloc_printf("<jemalloc>:utrace: %d %p = malloc(%zu)\n",
		    getpid(), ut->r, ut->s);
	} else if (ut->p != NULL && ut->r != NULL) {
		malloc_printf("<jemalloc>:utrace: %d %p = realloc(%p, %zu)\n",
		    getpid(), ut->r, ut->p, ut->s);
	} else
		malloc_printf("<jemalloc>:utrace: %d free(%p)\n", getpid(),
		    ut->p);

	return (0);
}
#endif

#ifdef JEMALLOC_STATS
/*
 * Print to stderr in such a way as to (hopefully) avoid memory allocation.
 */
static void
malloc_printf(const char *format, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	jemalloc_message(buf, "", "", "");
}
#endif

/******************************************************************************/
/*
 * Begin pthreads integration.  We intercept pthread_create() calls in order
 * to toggle isthreaded if the process goes multi-threaded.
 */

#ifdef JEMALLOC_LAZY_LOCK
int (*pthread_create_fptr)(pthread_t *, const pthread_attr_t *,
    void *(*)(void *), void *);

static void
get_pthread_create_fptr(void)
{

	pthread_create_fptr = dlsym(RTLD_NEXT, "pthread_create");
	if (pthread_create_fptr == NULL) {
		jemalloc_message("<jemalloc>",
		    ": Error in dlsym(RTLD_NEXT, \"pthread_create\")\n", "",
		    "");
		abort();
	}
}

int
pthread_create(pthread_t * thread, const pthread_attr_t * attr,
    void *(*start_routine)(void *), void * arg)
{
	static pthread_once_t once_control = PTHREAD_ONCE_INIT;

	pthread_once(&once_control, get_pthread_create_fptr);

	isthreaded = true;
	return pthread_create_fptr(thread, attr, start_routine, arg);
}
#endif

/******************************************************************************/
/*
 * Begin mutex.
 */

static bool
malloc_mutex_init(malloc_mutex_t *mutex)
{
	pthread_mutexattr_t attr;

	if (pthread_mutexattr_init(&attr) != 0)
		return (true);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
	if (pthread_mutex_init(mutex, &attr) != 0) {
		pthread_mutexattr_destroy(&attr);
		return (true);
	}
	pthread_mutexattr_destroy(&attr);

	return (false);
}

static inline void
malloc_mutex_lock(malloc_mutex_t *mutex)
{

	if (isthreaded)
		pthread_mutex_lock(mutex);
}

static inline void
malloc_mutex_unlock(malloc_mutex_t *mutex)
{

	if (isthreaded)
		pthread_mutex_unlock(mutex);
}

/*
 * End mutex.
 */
/******************************************************************************/
/*
 * Begin spin lock.  Spin locks here are actually adaptive mutexes that block
 * after a period of spinning, because unbounded spinning would allow for
 * priority inversion.
 */

static bool
malloc_spin_init(pthread_mutex_t *lock)
{

	if (pthread_mutex_init(lock, NULL) != 0)
		return (true);

	return (false);
}

static inline unsigned
malloc_spin_lock(pthread_mutex_t *lock)
{
	unsigned ret = 0;

	if (isthreaded) {
		if (pthread_mutex_trylock(lock) != 0) {
			/* Exponentially back off if there are multiple CPUs. */
			if (ncpus > 1) {
				unsigned i;
				volatile unsigned j;

				for (i = 1; i <= SPIN_LIMIT_2POW; i++) {
					for (j = 0; j < (1U << i); j++) {
						ret++;
						CPU_SPINWAIT;
					}

					if (pthread_mutex_trylock(lock) == 0)
						return (ret);
				}
			}

			/*
			 * Spinning failed.  Block until the lock becomes
			 * available, in order to avoid indefinite priority
			 * inversion.
			 */
			pthread_mutex_lock(lock);
			assert((ret << BLOCK_COST_2POW) != 0 || ncpus == 1);
			return (ret << BLOCK_COST_2POW);
		}
	}

	return (ret);
}

static inline void
malloc_spin_unlock(pthread_mutex_t *lock)
{

	if (isthreaded)
		pthread_mutex_unlock(lock);
}

/*
 * End spin lock.
 */
/******************************************************************************/
/*
 * Begin Utility functions/macros.
 */

/* Return the chunk address for allocation address a. */
#define	CHUNK_ADDR2BASE(a)						\
	((void *)((uintptr_t)(a) & ~chunksize_mask))

/* Return the chunk offset of address a. */
#define	CHUNK_ADDR2OFFSET(a)						\
	((size_t)((uintptr_t)(a) & chunksize_mask))

/* Return the smallest chunk multiple that is >= s. */
#define	CHUNK_CEILING(s)						\
	(((s) + chunksize_mask) & ~chunksize_mask)

/* Return the smallest quantum multiple that is >= a. */
#define	QUANTUM_CEILING(a)						\
	(((a) + QUANTUM_MASK) & ~QUANTUM_MASK)

/* Return the smallest cacheline multiple that is >= s. */
#define	CACHELINE_CEILING(s)						\
	(((s) + CACHELINE_MASK) & ~CACHELINE_MASK)

/* Return the smallest subpage multiple that is >= s. */
#define	SUBPAGE_CEILING(s)						\
	(((s) + SUBPAGE_MASK) & ~SUBPAGE_MASK)

/* Return the smallest pagesize multiple that is >= s. */
#define	PAGE_CEILING(s)							\
	(((s) + PAGE_MASK) & ~PAGE_MASK)

#ifdef JEMALLOC_TINY
/* Compute the smallest power of 2 that is >= x. */
static inline size_t
pow2_ceil(size_t x)
{

	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
#if (SIZEOF_PTR == 8)
	x |= x >> 32;
#endif
	x++;
	return (x);
}
#endif

#ifdef JEMALLOC_BALANCE
/*
 * Use a simple linear congruential pseudo-random number generator:
 *
 *   prn(y) = (a*x + c) % m
 *
 * where the following constants ensure maximal period:
 *
 *   a == Odd number (relatively prime to 2^n), and (a-1) is a multiple of 4.
 *   c == Odd number (relatively prime to 2^n).
 *   m == 2^32
 *
 * See Knuth's TAOCP 3rd Ed., Vol. 2, pg. 17 for details on these constraints.
 *
 * This choice of m has the disadvantage that the quality of the bits is
 * proportional to bit position.  For example. the lowest bit has a cycle of 2,
 * the next has a cycle of 4, etc.  For this reason, we prefer to use the upper
 * bits.
 */
#  define PRN_DEFINE(suffix, var, a, c)					\
static inline void							\
sprn_##suffix(uint32_t seed)						\
{									\
	var = seed;							\
}									\
									\
static inline uint32_t							\
prn_##suffix(uint32_t lg_range)						\
{									\
	uint32_t ret, x;						\
									\
	assert(lg_range > 0);						\
	assert(lg_range <= 32);						\
									\
	x = (var * (a)) + (c);						\
	var = x;							\
	ret = x >> (32 - lg_range);					\
									\
	return (ret);							\
}
#  define SPRN(suffix, seed)	sprn_##suffix(seed)
#  define PRN(suffix, lg_range)	prn_##suffix(lg_range)
#endif

#ifdef JEMALLOC_BALANCE
/* Define the PRNG used for arena assignment. */
static __thread uint32_t balance_x;
PRN_DEFINE(balance, balance_x, 1297, 1301)
#endif

/******************************************************************************/

#ifdef JEMALLOC_DSS
static bool
base_pages_alloc_dss(size_t minsize)
{

	/*
	 * Do special DSS allocation here, since base allocations don't need to
	 * be chunk-aligned.
	 */
	malloc_mutex_lock(&dss_mtx);
	if (dss_prev != (void *)-1) {
		intptr_t incr;
		size_t csize = CHUNK_CEILING(minsize);

		do {
			/* Get the current end of the DSS. */
			dss_max = sbrk(0);

			/*
			 * Calculate how much padding is necessary to
			 * chunk-align the end of the DSS.  Don't worry about
			 * dss_max not being chunk-aligned though.
			 */
			incr = (intptr_t)chunksize
			    - (intptr_t)CHUNK_ADDR2OFFSET(dss_max);
			assert(incr >= 0);
			if ((size_t)incr < minsize)
				incr += csize;

			dss_prev = sbrk(incr);
			if (dss_prev == dss_max) {
				/* Success. */
				dss_max = (void *)((intptr_t)dss_prev + incr);
				base_pages = dss_prev;
				base_next_addr = base_pages;
				base_past_addr = dss_max;
#ifdef JEMALLOC_STATS
				base_mapped += incr;
#endif
				malloc_mutex_unlock(&dss_mtx);
				return (false);
			}
		} while (dss_prev != (void *)-1);
	}
	malloc_mutex_unlock(&dss_mtx);

	return (true);
}
#endif

static bool
base_pages_alloc_mmap(size_t minsize)
{
	size_t csize;

	assert(minsize != 0);
	csize = PAGE_CEILING(minsize);
	base_pages = pages_map(NULL, csize);
	if (base_pages == NULL)
		return (true);
	base_next_addr = base_pages;
	base_past_addr = (void *)((uintptr_t)base_pages + csize);
#ifdef JEMALLOC_STATS
	base_mapped += csize;
#endif

	return (false);
}

static bool
base_pages_alloc(size_t minsize)
{

#ifdef JEMALLOC_DSS
	if (opt_dss) {
		if (base_pages_alloc_dss(minsize) == false)
			return (false);
	}

	if (opt_mmap && minsize != 0)
#endif
	{
		if (base_pages_alloc_mmap(minsize) == false)
			return (false);
	}

	return (true);
}

static void *
base_alloc(size_t size)
{
	void *ret;
	size_t csize;

	/* Round size up to nearest multiple of the cacheline size. */
	csize = CACHELINE_CEILING(size);

	malloc_mutex_lock(&base_mtx);
	/* Make sure there's enough space for the allocation. */
	if ((uintptr_t)base_next_addr + csize > (uintptr_t)base_past_addr) {
		if (base_pages_alloc(csize)) {
			malloc_mutex_unlock(&base_mtx);
			return (NULL);
		}
	}
	/* Allocate. */
	ret = base_next_addr;
	base_next_addr = (void *)((uintptr_t)base_next_addr + csize);
	malloc_mutex_unlock(&base_mtx);

	return (ret);
}

static void *
base_calloc(size_t number, size_t size)
{
	void *ret;

	ret = base_alloc(number * size);
	memset(ret, 0, number * size);

	return (ret);
}

static extent_node_t *
base_node_alloc(void)
{
	extent_node_t *ret;

	malloc_mutex_lock(&base_mtx);
	if (base_nodes != NULL) {
		ret = base_nodes;
		base_nodes = *(extent_node_t **)ret;
		malloc_mutex_unlock(&base_mtx);
	} else {
		malloc_mutex_unlock(&base_mtx);
		ret = (extent_node_t *)base_alloc(sizeof(extent_node_t));
	}

	return (ret);
}

static void
base_node_dealloc(extent_node_t *node)
{

	malloc_mutex_lock(&base_mtx);
	*(extent_node_t **)node = base_nodes;
	base_nodes = node;
	malloc_mutex_unlock(&base_mtx);
}

/******************************************************************************/

#ifdef JEMALLOC_STATS
static void
stats_print(arena_t *arena)
{
	unsigned i, gap_start;

	malloc_printf("dirty: %zu page%s dirty, %llu sweep%s,"
	    " %llu madvise%s, %llu page%s purged\n",
	    arena->ndirty, arena->ndirty == 1 ? "" : "s",
	    arena->stats.npurge, arena->stats.npurge == 1 ? "" : "s",
	    arena->stats.nmadvise, arena->stats.nmadvise == 1 ? "" : "s",
	    arena->stats.purged, arena->stats.purged == 1 ? "" : "s");

	malloc_printf("            allocated      nmalloc      ndalloc\n");
	malloc_printf("small:   %12zu %12llu %12llu\n",
	    arena->stats.allocated_small, arena->stats.nmalloc_small,
	    arena->stats.ndalloc_small);
	malloc_printf("large:   %12zu %12llu %12llu\n",
	    arena->stats.allocated_large, arena->stats.nmalloc_large,
	    arena->stats.ndalloc_large);
	malloc_printf("total:   %12zu %12llu %12llu\n",
	    arena->stats.allocated_small + arena->stats.allocated_large,
	    arena->stats.nmalloc_small + arena->stats.nmalloc_large,
	    arena->stats.ndalloc_small + arena->stats.ndalloc_large);
	malloc_printf("mapped:  %12zu\n", arena->stats.mapped);

#ifdef JEMALLOC_MAG
	if (opt_mag) {
		malloc_printf("bins:     bin   size regs pgs      mags   "
		    "newruns    reruns maxruns curruns\n");
	} else {
#endif
		malloc_printf("bins:     bin   size regs pgs  requests   "
		    "newruns    reruns maxruns curruns\n");
#ifdef JEMALLOC_MAG
	}
#endif
	for (i = 0, gap_start = UINT_MAX; i < nbins; i++) {
		if (arena->bins[i].stats.nruns == 0) {
			if (gap_start == UINT_MAX)
				gap_start = i;
		} else {
			if (gap_start != UINT_MAX) {
				if (i > gap_start + 1) {
					/* Gap of more than one size class. */
					malloc_printf("[%u..%u]\n",
					    gap_start, i - 1);
				} else {
					/* Gap of one size class. */
					malloc_printf("[%u]\n", gap_start);
				}
				gap_start = UINT_MAX;
			}
			malloc_printf(
			    "%13u %1s %4u %4u %3u %9llu %9llu"
			    " %9llu %7lu %7lu\n",
			    i,
			    i < ntbins ? "T" : i < ntbins + nqbins ? "Q" :
			    i < ntbins + nqbins + ncbins ? "C" : "S",
			    arena->bins[i].reg_size,
			    arena->bins[i].nregs,
			    arena->bins[i].run_size >> PAGE_SHIFT,
#ifdef JEMALLOC_MAG
			    (opt_mag) ? arena->bins[i].stats.nmags :
#endif
			    arena->bins[i].stats.nrequests,
			    arena->bins[i].stats.nruns,
			    arena->bins[i].stats.reruns,
			    arena->bins[i].stats.highruns,
			    arena->bins[i].stats.curruns);
		}
	}
	if (gap_start != UINT_MAX) {
		if (i > gap_start + 1) {
			/* Gap of more than one size class. */
			malloc_printf("[%u..%u]\n", gap_start, i - 1);
		} else {
			/* Gap of one size class. */
			malloc_printf("[%u]\n", gap_start);
		}
	}
}
#endif

/*
 * End Utility functions/macros.
 */
/******************************************************************************/
/*
 * Begin extent tree code.
 */

#ifdef JEMALLOC_DSS
static inline int
extent_szad_comp(extent_node_t *a, extent_node_t *b)
{
	int ret;
	size_t a_size = a->size;
	size_t b_size = b->size;

	ret = (a_size > b_size) - (a_size < b_size);
	if (ret == 0) {
		uintptr_t a_addr = (uintptr_t)a->addr;
		uintptr_t b_addr = (uintptr_t)b->addr;

		ret = (a_addr > b_addr) - (a_addr < b_addr);
	}

	return (ret);
}

/* Wrap red-black tree macros in functions. */
rb_wrap(static, extent_tree_szad_, extent_tree_t, extent_node_t,
    link_szad, extent_szad_comp)
#endif

static inline int
extent_ad_comp(extent_node_t *a, extent_node_t *b)
{
	uintptr_t a_addr = (uintptr_t)a->addr;
	uintptr_t b_addr = (uintptr_t)b->addr;

	return ((a_addr > b_addr) - (a_addr < b_addr));
}

/* Wrap red-black tree macros in functions. */
rb_wrap(static, extent_tree_ad_, extent_tree_t, extent_node_t, link_ad,
    extent_ad_comp)

/*
 * End extent tree code.
 */
/******************************************************************************/
/*
 * Begin chunk management functions.
 */

static void *
pages_map(void *addr, size_t size)
{
	void *ret;

	/*
	 * We don't use MAP_FIXED here, because it can cause the *replacement*
	 * of existing mappings, and we only want to create new mappings.
	 */
	ret = mmap(addr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON,
	    -1, 0);
	assert(ret != NULL);

	if (ret == MAP_FAILED)
		ret = NULL;
	else if (addr != NULL && ret != addr) {
		/*
		 * We succeeded in mapping memory, but not in the right place.
		 */
		if (munmap(ret, size) == -1) {
			char buf[STRERROR_BUF];

			strerror_r(errno, buf, sizeof(buf));
			jemalloc_message("<jemalloc>",
			    ": Error in munmap(): ", buf, "\n");
			if (opt_abort)
				abort();
		}
		ret = NULL;
	}

	assert(ret == NULL || (addr == NULL && ret != addr)
	    || (addr != NULL && ret == addr));
	return (ret);
}

static void
pages_unmap(void *addr, size_t size)
{

	if (munmap(addr, size) == -1) {
		char buf[STRERROR_BUF];

		strerror_r(errno, buf, sizeof(buf));
		jemalloc_message("<jemalloc>",
		    ": Error in munmap(): ", buf, "\n");
		if (opt_abort)
			abort();
	}
}

#ifdef JEMALLOC_DSS
static void *
chunk_alloc_dss(size_t size)
{

	/*
	 * sbrk() uses a signed increment argument, so take care not to
	 * interpret a huge allocation request as a negative increment.
	 */
	if ((intptr_t)size < 0)
		return (NULL);

	malloc_mutex_lock(&dss_mtx);
	if (dss_prev != (void *)-1) {
		intptr_t incr;

		/*
		 * The loop is necessary to recover from races with other
		 * threads that are using the DSS for something other than
		 * malloc.
		 */
		do {
			void *ret;

			/* Get the current end of the DSS. */
			dss_max = sbrk(0);

			/*
			 * Calculate how much padding is necessary to
			 * chunk-align the end of the DSS.
			 */
			incr = (intptr_t)size
			    - (intptr_t)CHUNK_ADDR2OFFSET(dss_max);
			if (incr == (intptr_t)size)
				ret = dss_max;
			else {
				ret = (void *)((intptr_t)dss_max + incr);
				incr += size;
			}

			dss_prev = sbrk(incr);
			if (dss_prev == dss_max) {
				/* Success. */
				dss_max = (void *)((intptr_t)dss_prev + incr);
				malloc_mutex_unlock(&dss_mtx);
				return (ret);
			}
		} while (dss_prev != (void *)-1);
	}
	malloc_mutex_unlock(&dss_mtx);

	return (NULL);
}

static void *
chunk_recycle_dss(size_t size, bool zero)
{
	extent_node_t *node, key;

	key.addr = NULL;
	key.size = size;
	malloc_mutex_lock(&dss_mtx);
	node = extent_tree_szad_nsearch(&dss_chunks_szad, &key);
	if (node != NULL) {
		void *ret = node->addr;

		/* Remove node from the tree. */
		extent_tree_szad_remove(&dss_chunks_szad, node);
		if (node->size == size) {
			extent_tree_ad_remove(&dss_chunks_ad, node);
			base_node_dealloc(node);
		} else {
			/*
			 * Insert the remainder of node's address range as a
			 * smaller chunk.  Its position within dss_chunks_ad
			 * does not change.
			 */
			assert(node->size > size);
			node->addr = (void *)((uintptr_t)node->addr + size);
			node->size -= size;
			extent_tree_szad_insert(&dss_chunks_szad, node);
		}
		malloc_mutex_unlock(&dss_mtx);

		if (zero)
			memset(ret, 0, size);
		return (ret);
	}
	malloc_mutex_unlock(&dss_mtx);

	return (NULL);
}
#endif

static void *
chunk_alloc_mmap(size_t size)
{
	void *ret;
	size_t offset;

	/*
	 * Ideally, there would be a way to specify alignment to mmap() (like
	 * NetBSD has), but in the absence of such a feature, we have to work
	 * hard to efficiently create aligned mappings.  The reliable, but
	 * expensive method is to create a mapping that is over-sized, then
	 * trim the excess.  However, that always results in at least one call
	 * to pages_unmap().
	 *
	 * A more optimistic approach is to try mapping precisely the right
	 * amount, then try to append another mapping if alignment is off.  In
	 * practice, this works out well as long as the application is not
	 * interleaving mappings via direct mmap() calls.  If we do run into a
	 * situation where there is an interleaved mapping and we are unable to
	 * extend an unaligned mapping, our best option is to momentarily
	 * revert to the reliable-but-expensive method.  This will tend to
	 * leave a gap in the memory map that is too small to cause later
	 * problems for the optimistic method.
	 */

	ret = pages_map(NULL, size);
	if (ret == NULL)
		return (NULL);

	offset = CHUNK_ADDR2OFFSET(ret);
	if (offset != 0) {
		/* Try to extend chunk boundary. */
		if (pages_map((void *)((uintptr_t)ret + size),
		    chunksize - offset) == NULL) {
			/*
			 * Extension failed.  Clean up, then revert to the
			 * reliable-but-expensive method.
			 */
			pages_unmap(ret, size);

			/* Beware size_t wrap-around. */
			if (size + chunksize <= size)
				return NULL;

			ret = pages_map(NULL, size + chunksize);
			if (ret == NULL)
				return (NULL);

			/* Clean up unneeded leading/trailing space. */
			offset = CHUNK_ADDR2OFFSET(ret);
			if (offset != 0) {
				/* Leading space. */
				pages_unmap(ret, chunksize - offset);

				ret = (void *)((uintptr_t)ret +
				    (chunksize - offset));

				/* Trailing space. */
				pages_unmap((void *)((uintptr_t)ret + size),
				    offset);
			} else {
				/* Trailing space only. */
				pages_unmap((void *)((uintptr_t)ret + size),
				    chunksize);
			}
		} else {
			/* Clean up unneeded leading space. */
			pages_unmap(ret, chunksize - offset);
			ret = (void *)((uintptr_t)ret + (chunksize - offset));
		}
	}

	return (ret);
}

static void *
chunk_alloc(size_t size, bool zero)
{
	void *ret;

	assert(size != 0);
	assert((size & chunksize_mask) == 0);

#ifdef JEMALLOC_DSS
	if (opt_dss) {
		ret = chunk_recycle_dss(size, zero);
		if (ret != NULL) {
			goto RETURN;
		}

		ret = chunk_alloc_dss(size);
		if (ret != NULL)
			goto RETURN;
	}

	if (opt_mmap)
#endif
	{
		ret = chunk_alloc_mmap(size);
		if (ret != NULL)
			goto RETURN;
	}

	/* All strategies for allocation failed. */
	ret = NULL;
RETURN:
#ifdef JEMALLOC_STATS
	if (ret != NULL) {
		stats_chunks.nchunks += (size / chunksize);
		stats_chunks.curchunks += (size / chunksize);
	}
	if (stats_chunks.curchunks > stats_chunks.highchunks)
		stats_chunks.highchunks = stats_chunks.curchunks;
#endif

	assert(CHUNK_ADDR2BASE(ret) == ret);
	return (ret);
}

#ifdef JEMALLOC_DSS
static extent_node_t *
chunk_dealloc_dss_record(void *chunk, size_t size)
{
	extent_node_t *node, *prev, key;

	key.addr = (void *)((uintptr_t)chunk + size);
	node = extent_tree_ad_nsearch(&dss_chunks_ad, &key);
	/* Try to coalesce forward. */
	if (node != NULL && node->addr == key.addr) {
		/*
		 * Coalesce chunk with the following address range.  This does
		 * not change the position within dss_chunks_ad, so only
		 * remove/insert from/into dss_chunks_szad.
		 */
		extent_tree_szad_remove(&dss_chunks_szad, node);
		node->addr = chunk;
		node->size += size;
		extent_tree_szad_insert(&dss_chunks_szad, node);
	} else {
		/*
		 * Coalescing forward failed, so insert a new node.  Drop
		 * dss_mtx during node allocation, since it is possible that a
		 * new base chunk will be allocated.
		 */
		malloc_mutex_unlock(&dss_mtx);
		node = base_node_alloc();
		malloc_mutex_lock(&dss_mtx);
		if (node == NULL)
			return (NULL);
		node->addr = chunk;
		node->size = size;
		extent_tree_ad_insert(&dss_chunks_ad, node);
		extent_tree_szad_insert(&dss_chunks_szad, node);
	}

	/* Try to coalesce backward. */
	prev = extent_tree_ad_prev(&dss_chunks_ad, node);
	if (prev != NULL && (void *)((uintptr_t)prev->addr + prev->size) ==
	    chunk) {
		/*
		 * Coalesce chunk with the previous address range.  This does
		 * not change the position within dss_chunks_ad, so only
		 * remove/insert node from/into dss_chunks_szad.
		 */
		extent_tree_szad_remove(&dss_chunks_szad, prev);
		extent_tree_ad_remove(&dss_chunks_ad, prev);

		extent_tree_szad_remove(&dss_chunks_szad, node);
		node->addr = prev->addr;
		node->size += prev->size;
		extent_tree_szad_insert(&dss_chunks_szad, node);

		base_node_dealloc(prev);
	}

	return (node);
}

static bool
chunk_dealloc_dss(void *chunk, size_t size)
{

	malloc_mutex_lock(&dss_mtx);
	if ((uintptr_t)chunk >= (uintptr_t)dss_base
	    && (uintptr_t)chunk < (uintptr_t)dss_max) {
		extent_node_t *node;

		/* Try to coalesce with other unused chunks. */
		node = chunk_dealloc_dss_record(chunk, size);
		if (node != NULL) {
			chunk = node->addr;
			size = node->size;
		}

		/* Get the current end of the DSS. */
		dss_max = sbrk(0);

		/*
		 * Try to shrink the DSS if this chunk is at the end of the
		 * DSS.  The sbrk() call here is subject to a race condition
		 * with threads that use brk(2) or sbrk(2) directly, but the
		 * alternative would be to leak memory for the sake of poorly
		 * designed multi-threaded programs.
		 */
		if ((void *)((uintptr_t)chunk + size) == dss_max
		    && (dss_prev = sbrk(-(intptr_t)size)) == dss_max) {
			/* Success. */
			dss_max = (void *)((intptr_t)dss_prev - (intptr_t)size);

			if (node != NULL) {
				extent_tree_szad_remove(&dss_chunks_szad, node);
				extent_tree_ad_remove(&dss_chunks_ad, node);
				base_node_dealloc(node);
			}
			malloc_mutex_unlock(&dss_mtx);
		} else {
			malloc_mutex_unlock(&dss_mtx);
			madvise(chunk, size, MADV_DONTNEED);
		}

		return (false);
	}
	malloc_mutex_unlock(&dss_mtx);

	return (true);
}
#endif

static void
chunk_dealloc_mmap(void *chunk, size_t size)
{

	pages_unmap(chunk, size);
}

static void
chunk_dealloc(void *chunk, size_t size)
{

	assert(chunk != NULL);
	assert(CHUNK_ADDR2BASE(chunk) == chunk);
	assert(size != 0);
	assert((size & chunksize_mask) == 0);

#ifdef JEMALLOC_STATS
	stats_chunks.curchunks -= (size / chunksize);
#endif

#ifdef JEMALLOC_DSS
	if (opt_dss) {
		if (chunk_dealloc_dss(chunk, size) == false)
			return;
	}

	if (opt_mmap)
#endif
		chunk_dealloc_mmap(chunk, size);
}

/*
 * End chunk management functions.
 */
/******************************************************************************/
/*
 * Begin arena.
 */

/*
 * Choose an arena based on a per-thread value (fast-path code, calls slow-path
 * code if necessary).
 */
static inline arena_t *
choose_arena(void)
{
	arena_t *ret;

	/*
	 * We can only use TLS if this is a PIC library, since for the static
	 * library version, libc's malloc is used by TLS allocation, which
	 * introduces a bootstrapping issue.
	 */
#ifndef NO_TLS
	ret = arenas_map;
	if (ret == NULL) {
		ret = choose_arena_hard();
		assert(ret != NULL);
	}
#else
	if (isthreaded && narenas > 1) {
		unsigned long ind;

		/*
		 * Hash pthread_self() to one of the arenas.  There is a prime
		 * number of arenas, so this has a reasonable chance of
		 * working.  Even so, the hashing can be easily thwarted by
		 * inconvenient pthread_self() values.  Without specific
		 * knowledge of how pthread_self() calculates values, we can't
		 * easily do much better than this.
		 */
		ind = (unsigned long) pthread_self() % narenas;

		/*
		 * Optimistially assume that arenas[ind] has been initialized.
		 * At worst, we find out that some other thread has already
		 * done so, after acquiring the lock in preparation.  Note that
		 * this lazy locking also has the effect of lazily forcing
		 * cache coherency; without the lock acquisition, there's no
		 * guarantee that modification of arenas[ind] by another thread
		 * would be seen on this CPU for an arbitrary amount of time.
		 *
		 * In general, this approach to modifying a synchronized value
		 * isn't a good idea, but in this case we only ever modify the
		 * value once, so things work out well.
		 */
		ret = arenas[ind];
		if (ret == NULL) {
			/*
			 * Avoid races with another thread that may have already
			 * initialized arenas[ind].
			 */
			malloc_spin_lock(&arenas_lock);
			if (arenas[ind] == NULL)
				ret = arenas_extend((unsigned)ind);
			else
				ret = arenas[ind];
			malloc_spin_unlock(&arenas_lock);
		}
	} else
		ret = arenas[0];
#endif

	assert(ret != NULL);
	return (ret);
}

#ifndef NO_TLS
/*
 * Choose an arena based on a per-thread value (slow-path code only, called
 * only by choose_arena()).
 */
static arena_t *
choose_arena_hard(void)
{
	arena_t *ret;

	assert(isthreaded);

#ifdef JEMALLOC_BALANCE
	/* Seed the PRNG used for arena load balancing. */
	SPRN(balance, (uint32_t)(uintptr_t)(pthread_self()));
#endif

	if (narenas > 1) {
#ifdef JEMALLOC_BALANCE
		unsigned ind;

		ind = PRN(balance, narenas_2pow);
		if ((ret = arenas[ind]) == NULL) {
			malloc_spin_lock(&arenas_lock);
			if ((ret = arenas[ind]) == NULL)
				ret = arenas_extend(ind);
			malloc_spin_unlock(&arenas_lock);
		}
#else
		malloc_spin_lock(&arenas_lock);
		if ((ret = arenas[next_arena]) == NULL)
			ret = arenas_extend(next_arena);
		next_arena = (next_arena + 1) % narenas;
		malloc_spin_unlock(&arenas_lock);
#endif
	} else
		ret = arenas[0];

	arenas_map = ret;

	return (ret);
}
#endif

static inline int
arena_chunk_comp(arena_chunk_t *a, arena_chunk_t *b)
{
	uintptr_t a_chunk = (uintptr_t)a;
	uintptr_t b_chunk = (uintptr_t)b;

	assert(a != NULL);
	assert(b != NULL);

	return ((a_chunk > b_chunk) - (a_chunk < b_chunk));
}

/* Wrap red-black tree macros in functions. */
rb_wrap(static, arena_chunk_tree_dirty_, arena_chunk_tree_t,
    arena_chunk_t, link_dirty, arena_chunk_comp)

static inline int
arena_run_comp(arena_chunk_map_t *a, arena_chunk_map_t *b)
{
	uintptr_t a_mapelm = (uintptr_t)a;
	uintptr_t b_mapelm = (uintptr_t)b;

	assert(a != NULL);
	assert(b != NULL);

	return ((a_mapelm > b_mapelm) - (a_mapelm < b_mapelm));
}

/* Wrap red-black tree macros in functions. */
rb_wrap(static, arena_run_tree_, arena_run_tree_t, arena_chunk_map_t,
    link, arena_run_comp)

static inline int
arena_avail_comp(arena_chunk_map_t *a, arena_chunk_map_t *b)
{
	int ret;
	size_t a_size = a->bits & ~PAGE_MASK;
	size_t b_size = b->bits & ~PAGE_MASK;

	ret = (a_size > b_size) - (a_size < b_size);
	if (ret == 0) {
		uintptr_t a_mapelm, b_mapelm;

		if ((a->bits & CHUNK_MAP_KEY) == 0)
			a_mapelm = (uintptr_t)a;
		else {
			/*
			 * Treat keys as though they are lower than anything
			 * else.
			 */
			a_mapelm = 0;
		}
		b_mapelm = (uintptr_t)b;

		ret = (a_mapelm > b_mapelm) - (a_mapelm < b_mapelm);
	}

	return (ret);
}

/* Wrap red-black tree macros in functions. */
rb_wrap(static, arena_avail_tree_, arena_avail_tree_t,
    arena_chunk_map_t, link, arena_avail_comp)

static inline void *
arena_run_reg_alloc(arena_run_t *run, arena_bin_t *bin)
{
	void *ret;
	unsigned i, mask, bit, regind;

	assert(run->magic == ARENA_RUN_MAGIC);
	assert(run->regs_minelm < bin->regs_mask_nelms);

	/*
	 * Move the first check outside the loop, so that run->regs_minelm can
	 * be updated unconditionally, without the possibility of updating it
	 * multiple times.
	 */
	i = run->regs_minelm;
	mask = run->regs_mask[i];
	if (mask != 0) {
		/* Usable allocation found. */
		bit = ffs((int)mask) - 1;

		regind = ((i << (SIZEOF_INT_2POW + 3)) + bit);
		assert(regind < bin->nregs);
		ret = (void *)(((uintptr_t)run) + bin->reg0_offset
		    + (bin->reg_size * regind));

		/* Clear bit. */
		mask ^= (1U << bit);
		run->regs_mask[i] = mask;

		return (ret);
	}

	for (i++; i < bin->regs_mask_nelms; i++) {
		mask = run->regs_mask[i];
		if (mask != 0) {
			/* Usable allocation found. */
			bit = ffs((int)mask) - 1;

			regind = ((i << (SIZEOF_INT_2POW + 3)) + bit);
			assert(regind < bin->nregs);
			ret = (void *)(((uintptr_t)run) + bin->reg0_offset
			    + (bin->reg_size * regind));

			/* Clear bit. */
			mask ^= (1U << bit);
			run->regs_mask[i] = mask;

			/*
			 * Make a note that nothing before this element
			 * contains a free region.
			 */
			run->regs_minelm = i; /* Low payoff: + (mask == 0); */

			return (ret);
		}
	}
	/* Not reached. */
	assert(0);
	return (NULL);
}

static inline void
arena_run_reg_dalloc(arena_run_t *run, arena_bin_t *bin, void *ptr, size_t size)
{
	unsigned diff, regind, elm, bit;

	assert(run->magic == ARENA_RUN_MAGIC);

	/*
	 * Avoid doing division with a variable divisor if possible.  Using
	 * actual division here can reduce allocator throughput by over 20%!
	 */
	diff = (unsigned)((uintptr_t)ptr - (uintptr_t)run - bin->reg0_offset);
	if ((size & (size - 1)) == 0) {
		/*
		 * log2_table allows fast division of a power of two in the
		 * [1..128] range.
		 *
		 * (x / divisor) becomes (x >> log2_table[divisor - 1]).
		 */
		static const unsigned char log2_table[] = {
		    0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7
		};

		if (size <= 128)
			regind = (diff >> log2_table[size - 1]);
		else if (size <= 32768)
			regind = diff >> (8 + log2_table[(size >> 8) - 1]);
		else
			regind = diff / size;
	} else if (size < qspace_max) {
		/*
		 * To divide by a number D that is not a power of two we
		 * multiply by (2^21 / D) and then right shift by 21 positions.
		 *
		 *   X / D
		 *
		 * becomes
		 *
		 *   (X * qsize_invs[(D >> QUANTUM_2POW) - 3])
		 *       >> SIZE_INV_SHIFT
		 *
		 * We can omit the first three elements, because we never
		 * divide by 0, and QUANTUM and 2*QUANTUM are both powers of
		 * two, which are handled above.
		 */
#define	SIZE_INV_SHIFT 21
#define	QSIZE_INV(s) (((1U << SIZE_INV_SHIFT) / (s << QUANTUM_2POW)) + 1)
		static const unsigned qsize_invs[] = {
		    QSIZE_INV(3),
		    QSIZE_INV(4), QSIZE_INV(5), QSIZE_INV(6), QSIZE_INV(7)
#if (QUANTUM_2POW < 4)
		    ,
		    QSIZE_INV(8), QSIZE_INV(9), QSIZE_INV(10), QSIZE_INV(11),
		    QSIZE_INV(12),QSIZE_INV(13), QSIZE_INV(14), QSIZE_INV(15)
#endif
		};
		assert(QUANTUM * (((sizeof(qsize_invs)) / sizeof(unsigned)) + 3)
		    >= (1U << QSPACE_MAX_2POW_DEFAULT));

		if (size <= (((sizeof(qsize_invs) / sizeof(unsigned)) + 2) <<
		    QUANTUM_2POW)) {
			regind = qsize_invs[(size >> QUANTUM_2POW) - 3] * diff;
			regind >>= SIZE_INV_SHIFT;
		} else
			regind = diff / size;
#undef QSIZE_INV
	} else if (size < cspace_max) {
#define	CSIZE_INV(s) (((1U << SIZE_INV_SHIFT) / (s << CACHELINE_2POW)) + 1)
		static const unsigned csize_invs[] = {
		    CSIZE_INV(3),
		    CSIZE_INV(4), CSIZE_INV(5), CSIZE_INV(6), CSIZE_INV(7)
		};
		assert(CACHELINE * (((sizeof(csize_invs)) / sizeof(unsigned)) +
		    3) >= (1U << CSPACE_MAX_2POW_DEFAULT));

		if (size <= (((sizeof(csize_invs) / sizeof(unsigned)) + 2) <<
		    CACHELINE_2POW)) {
			regind = csize_invs[(size >> CACHELINE_2POW) - 3] *
			    diff;
			regind >>= SIZE_INV_SHIFT;
		} else
			regind = diff / size;
#undef CSIZE_INV
	} else {
#define	SSIZE_INV(s) (((1U << SIZE_INV_SHIFT) / (s << SUBPAGE_2POW)) + 1)
		static const unsigned ssize_invs[] = {
		    SSIZE_INV(3),
		    SSIZE_INV(4), SSIZE_INV(5), SSIZE_INV(6), SSIZE_INV(7),
		    SSIZE_INV(8), SSIZE_INV(9), SSIZE_INV(10), SSIZE_INV(11),
		    SSIZE_INV(12), SSIZE_INV(13), SSIZE_INV(14), SSIZE_INV(15)
#if (STATIC_PAGE_SHIFT == 13)
		    ,
		    SSIZE_INV(16), SSIZE_INV(17), SSIZE_INV(18), SSIZE_INV(19),
		    SSIZE_INV(20), SSIZE_INV(21), SSIZE_INV(22), SSIZE_INV(23),
		    SSIZE_INV(24), SSIZE_INV(25), SSIZE_INV(26), SSIZE_INV(27),
		    SSIZE_INV(28), SSIZE_INV(29), SSIZE_INV(29), SSIZE_INV(30)
#endif
		};
		assert(SUBPAGE * (((sizeof(ssize_invs)) / sizeof(unsigned)) + 3)
		    >= PAGE_SIZE);

		if (size < (((sizeof(ssize_invs) / sizeof(unsigned)) + 2) <<
		    SUBPAGE_2POW)) {
			regind = ssize_invs[(size >> SUBPAGE_2POW) - 3] * diff;
			regind >>= SIZE_INV_SHIFT;
		} else
			regind = diff / size;
#undef SSIZE_INV
	}
#undef SIZE_INV_SHIFT
	assert(diff == regind * size);
	assert(regind < bin->nregs);

	elm = regind >> (SIZEOF_INT_2POW + 3);
	if (elm < run->regs_minelm)
		run->regs_minelm = elm;
	bit = regind - (elm << (SIZEOF_INT_2POW + 3));
	assert((run->regs_mask[elm] & (1U << bit)) == 0);
	run->regs_mask[elm] |= (1U << bit);
}

static void
arena_run_split(arena_t *arena, arena_run_t *run, size_t size, bool large,
    bool zero)
{
	arena_chunk_t *chunk;
	size_t old_ndirty, run_ind, total_pages, need_pages, rem_pages, i;

	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(run);
	old_ndirty = chunk->ndirty;
	run_ind = (unsigned)(((uintptr_t)run - (uintptr_t)chunk)
	    >> PAGE_SHIFT);
	total_pages = (chunk->map[run_ind].bits & ~PAGE_MASK) >>
	    PAGE_SHIFT;
	need_pages = (size >> PAGE_SHIFT);
	assert(need_pages > 0);
	assert(need_pages <= total_pages);
	rem_pages = total_pages - need_pages;

	arena_avail_tree_remove(&arena->runs_avail, &chunk->map[run_ind]);

	/* Keep track of trailing unused pages for later use. */
	if (rem_pages > 0) {
		chunk->map[run_ind+need_pages].bits = (rem_pages <<
		    PAGE_SHIFT) | (chunk->map[run_ind+need_pages].bits &
		    PAGE_MASK);
		chunk->map[run_ind+total_pages-1].bits = (rem_pages <<
		    PAGE_SHIFT) | (chunk->map[run_ind+total_pages-1].bits &
		    PAGE_MASK);
		arena_avail_tree_insert(&arena->runs_avail,
		    &chunk->map[run_ind+need_pages]);
	}

	for (i = 0; i < need_pages; i++) {
		/* Zero if necessary. */
		if (zero) {
			if ((chunk->map[run_ind + i].bits & CHUNK_MAP_ZEROED)
			    == 0) {
				memset((void *)((uintptr_t)chunk + ((run_ind
				    + i) << PAGE_SHIFT)), 0, PAGE_SIZE);
				/* CHUNK_MAP_ZEROED is cleared below. */
			}
		}

		/* Update dirty page accounting. */
		if (chunk->map[run_ind + i].bits & CHUNK_MAP_DIRTY) {
			chunk->ndirty--;
			arena->ndirty--;
			/* CHUNK_MAP_DIRTY is cleared below. */
		}

		/* Initialize the chunk map. */
		if (large) {
			chunk->map[run_ind + i].bits = CHUNK_MAP_LARGE
			    | CHUNK_MAP_ALLOCATED;
		} else {
			chunk->map[run_ind + i].bits = (size_t)run
			    | CHUNK_MAP_ALLOCATED;
		}
	}

	/*
	 * Set the run size only in the first element for large runs.  This is
	 * primarily a debugging aid, since the lack of size info for trailing
	 * pages only matters if the application tries to operate on an
	 * interior pointer.
	 */
	if (large)
		chunk->map[run_ind].bits |= size;

	if (chunk->ndirty == 0 && old_ndirty > 0)
		arena_chunk_tree_dirty_remove(&arena->chunks_dirty, chunk);
}

static arena_chunk_t *
arena_chunk_alloc(arena_t *arena)
{
	arena_chunk_t *chunk;
	size_t i;

	if (arena->spare != NULL) {
		chunk = arena->spare;
		arena->spare = NULL;
	} else {
		chunk = (arena_chunk_t *)chunk_alloc(chunksize, true);
		if (chunk == NULL)
			return (NULL);
#ifdef JEMALLOC_STATS
		arena->stats.mapped += chunksize;
#endif

		chunk->arena = arena;

		/*
		 * Claim that no pages are in use, since the header is merely
		 * overhead.
		 */
		chunk->ndirty = 0;

		/*
		 * Initialize the map to contain one maximal free untouched run.
		 */
		for (i = 0; i < arena_chunk_header_npages; i++)
			chunk->map[i].bits = 0;
		chunk->map[i].bits = arena_maxclass | CHUNK_MAP_ZEROED;
		for (i++; i < chunk_npages-1; i++) {
			chunk->map[i].bits = CHUNK_MAP_ZEROED;
		}
		chunk->map[chunk_npages-1].bits = arena_maxclass |
		    CHUNK_MAP_ZEROED;
	}

	/* Insert the run into the runs_avail tree. */
	arena_avail_tree_insert(&arena->runs_avail,
	    &chunk->map[arena_chunk_header_npages]);

	return (chunk);
}

static void
arena_chunk_dealloc(arena_t *arena, arena_chunk_t *chunk)
{

	if (arena->spare != NULL) {
		if (arena->spare->ndirty > 0) {
			arena_chunk_tree_dirty_remove(
			    &chunk->arena->chunks_dirty, arena->spare);
			arena->ndirty -= arena->spare->ndirty;
		}
		chunk_dealloc((void *)arena->spare, chunksize);
#ifdef JEMALLOC_STATS
		arena->stats.mapped -= chunksize;
#endif
	}

	/*
	 * Remove run from runs_avail, regardless of whether this chunk
	 * will be cached, so that the arena does not use it.  Dirty page
	 * flushing only uses the chunks_dirty tree, so leaving this chunk in
	 * the chunks_* trees is sufficient for that purpose.
	 */
	arena_avail_tree_remove(&arena->runs_avail,
	    &chunk->map[arena_chunk_header_npages]);

	arena->spare = chunk;
}

static arena_run_t *
arena_run_alloc(arena_t *arena, size_t size, bool large, bool zero)
{
	arena_chunk_t *chunk;
	arena_run_t *run;
	arena_chunk_map_t *mapelm, key;

	assert(size <= arena_maxclass);
	assert((size & PAGE_MASK) == 0);

	/* Search the arena's chunks for the lowest best fit. */
	key.bits = size | CHUNK_MAP_KEY;
	mapelm = arena_avail_tree_nsearch(&arena->runs_avail, &key);
	if (mapelm != NULL) {
		arena_chunk_t *run_chunk = CHUNK_ADDR2BASE(mapelm);
		size_t pageind = ((uintptr_t)mapelm - (uintptr_t)run_chunk->map)
		    / sizeof(arena_chunk_map_t);

		run = (arena_run_t *)((uintptr_t)run_chunk + (pageind
		    << PAGE_SHIFT));
		arena_run_split(arena, run, size, large, zero);
		return (run);
	}

	/*
	 * No usable runs.  Create a new chunk from which to allocate the run.
	 */
	chunk = arena_chunk_alloc(arena);
	if (chunk == NULL)
		return (NULL);
	run = (arena_run_t *)((uintptr_t)chunk + (arena_chunk_header_npages <<
	    PAGE_SHIFT));
	/* Update page map. */
	arena_run_split(arena, run, size, large, zero);
	return (run);
}

static void
arena_purge(arena_t *arena)
{
	arena_chunk_t *chunk;
	size_t i, npages;
#ifdef JEMALLOC_DEBUG
	size_t ndirty = 0;

	rb_foreach_begin(arena_chunk_t, link_dirty, &arena->chunks_dirty,
	    chunk) {
		ndirty += chunk->ndirty;
	} rb_foreach_end(arena_chunk_t, link_dirty, &arena->chunks_dirty, chunk)
	assert(ndirty == arena->ndirty);
#endif
	assert(arena->ndirty > opt_dirty_max);

#ifdef JEMALLOC_STATS
	arena->stats.npurge++;
#endif

	/*
	 * Iterate downward through chunks until enough dirty memory has been
	 * purged.  Terminate as soon as possible in order to minimize the
	 * number of system calls, even if a chunk has only been partially
	 * purged.
	 */
	while (arena->ndirty > (opt_dirty_max >> 1)) {
		chunk = arena_chunk_tree_dirty_last(&arena->chunks_dirty);
		assert(chunk != NULL);

		for (i = chunk_npages - 1; chunk->ndirty > 0; i--) {
			assert(i >= arena_chunk_header_npages);

			if (chunk->map[i].bits & CHUNK_MAP_DIRTY) {
				chunk->map[i].bits ^= CHUNK_MAP_DIRTY;
				/* Find adjacent dirty run(s). */
				for (npages = 1; i > arena_chunk_header_npages
				    && (chunk->map[i - 1].bits &
				    CHUNK_MAP_DIRTY); npages++) {
					i--;
					chunk->map[i].bits ^= CHUNK_MAP_DIRTY;
				}
				chunk->ndirty -= npages;
				arena->ndirty -= npages;

				madvise((void *)((uintptr_t)chunk + (i <<
				    PAGE_SHIFT)), (npages << PAGE_SHIFT),
				    MADV_DONTNEED);
#ifdef JEMALLOC_STATS
				arena->stats.nmadvise++;
				arena->stats.purged += npages;
#endif
				if (arena->ndirty <= (opt_dirty_max >> 1))
					break;
			}
		}

		if (chunk->ndirty == 0) {
			arena_chunk_tree_dirty_remove(&arena->chunks_dirty,
			    chunk);
		}
	}
}

static void
arena_run_dalloc(arena_t *arena, arena_run_t *run, bool dirty)
{
	arena_chunk_t *chunk;
	size_t size, run_ind, run_pages;

	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(run);
	run_ind = (size_t)(((uintptr_t)run - (uintptr_t)chunk)
	    >> PAGE_SHIFT);
	assert(run_ind >= arena_chunk_header_npages);
	assert(run_ind < chunk_npages);
	if ((chunk->map[run_ind].bits & CHUNK_MAP_LARGE) != 0)
		size = chunk->map[run_ind].bits & ~PAGE_MASK;
	else
		size = run->bin->run_size;
	run_pages = (size >> PAGE_SHIFT);

	/* Mark pages as unallocated in the chunk map. */
	if (dirty) {
		size_t i;

		for (i = 0; i < run_pages; i++) {
			assert((chunk->map[run_ind + i].bits & CHUNK_MAP_DIRTY)
			    == 0);
			chunk->map[run_ind + i].bits = CHUNK_MAP_DIRTY;
		}

		if (chunk->ndirty == 0) {
			arena_chunk_tree_dirty_insert(&arena->chunks_dirty,
			    chunk);
		}
		chunk->ndirty += run_pages;
		arena->ndirty += run_pages;
	} else {
		size_t i;

		for (i = 0; i < run_pages; i++) {
			chunk->map[run_ind + i].bits &= ~(CHUNK_MAP_LARGE |
			    CHUNK_MAP_ALLOCATED);
		}
	}
	chunk->map[run_ind].bits = size | (chunk->map[run_ind].bits &
	    PAGE_MASK);
	chunk->map[run_ind+run_pages-1].bits = size |
	    (chunk->map[run_ind+run_pages-1].bits & PAGE_MASK);

	/* Try to coalesce forward. */
	if (run_ind + run_pages < chunk_npages &&
	    (chunk->map[run_ind+run_pages].bits & CHUNK_MAP_ALLOCATED) == 0) {
		size_t nrun_size = chunk->map[run_ind+run_pages].bits &
		    ~PAGE_MASK;

		/*
		 * Remove successor from runs_avail; the coalesced run is
		 * inserted later.
		 */
		arena_avail_tree_remove(&arena->runs_avail,
		    &chunk->map[run_ind+run_pages]);

		size += nrun_size;
		run_pages = size >> PAGE_SHIFT;

		assert((chunk->map[run_ind+run_pages-1].bits & ~PAGE_MASK)
		    == nrun_size);
		chunk->map[run_ind].bits = size | (chunk->map[run_ind].bits &
		    PAGE_MASK);
		chunk->map[run_ind+run_pages-1].bits = size |
		    (chunk->map[run_ind+run_pages-1].bits & PAGE_MASK);
	}

	/* Try to coalesce backward. */
	if (run_ind > arena_chunk_header_npages && (chunk->map[run_ind-1].bits &
	    CHUNK_MAP_ALLOCATED) == 0) {
		size_t prun_size = chunk->map[run_ind-1].bits & ~PAGE_MASK;

		run_ind -= prun_size >> PAGE_SHIFT;

		/*
		 * Remove predecessor from runs_avail; the coalesced run is
		 * inserted later.
		 */
		arena_avail_tree_remove(&arena->runs_avail,
		    &chunk->map[run_ind]);

		size += prun_size;
		run_pages = size >> PAGE_SHIFT;

		assert((chunk->map[run_ind].bits & ~PAGE_MASK) ==
		    prun_size);
		chunk->map[run_ind].bits = size | (chunk->map[run_ind].bits &
		    PAGE_MASK);
		chunk->map[run_ind+run_pages-1].bits = size |
		    (chunk->map[run_ind+run_pages-1].bits & PAGE_MASK);
	}

	/* Insert into runs_avail, now that coalescing is complete. */
	arena_avail_tree_insert(&arena->runs_avail, &chunk->map[run_ind]);

	/* Deallocate chunk if it is now completely unused. */
	if ((chunk->map[arena_chunk_header_npages].bits & (~PAGE_MASK |
	    CHUNK_MAP_ALLOCATED)) == arena_maxclass)
		arena_chunk_dealloc(arena, chunk);

	/* Enforce opt_dirty_max. */
	if (arena->ndirty > opt_dirty_max)
		arena_purge(arena);
}

static void
arena_run_trim_head(arena_t *arena, arena_chunk_t *chunk, arena_run_t *run,
    size_t oldsize, size_t newsize)
{
	size_t pageind = ((uintptr_t)run - (uintptr_t)chunk) >> PAGE_SHIFT;
	size_t head_npages = (oldsize - newsize) >> PAGE_SHIFT;

	assert(oldsize > newsize);

	/*
	 * Update the chunk map so that arena_run_dalloc() can treat the
	 * leading run as separately allocated.
	 */
	chunk->map[pageind].bits = (oldsize - newsize) | CHUNK_MAP_LARGE |
	    CHUNK_MAP_ALLOCATED;
	chunk->map[pageind+head_npages].bits = newsize | CHUNK_MAP_LARGE |
	    CHUNK_MAP_ALLOCATED;

	arena_run_dalloc(arena, run, false);
}

static void
arena_run_trim_tail(arena_t *arena, arena_chunk_t *chunk, arena_run_t *run,
    size_t oldsize, size_t newsize, bool dirty)
{
	size_t pageind = ((uintptr_t)run - (uintptr_t)chunk) >> PAGE_SHIFT;
	size_t npages = newsize >> PAGE_SHIFT;

	assert(oldsize > newsize);

	/*
	 * Update the chunk map so that arena_run_dalloc() can treat the
	 * trailing run as separately allocated.
	 */
	chunk->map[pageind].bits = newsize | CHUNK_MAP_LARGE |
	    CHUNK_MAP_ALLOCATED;
	chunk->map[pageind+npages].bits = (oldsize - newsize) | CHUNK_MAP_LARGE
	    | CHUNK_MAP_ALLOCATED;

	arena_run_dalloc(arena, (arena_run_t *)((uintptr_t)run + newsize),
	    dirty);
}

static arena_run_t *
arena_bin_nonfull_run_get(arena_t *arena, arena_bin_t *bin)
{
	arena_chunk_map_t *mapelm;
	arena_run_t *run;
	unsigned i, remainder;

	/* Look for a usable run. */
	mapelm = arena_run_tree_first(&bin->runs);
	if (mapelm != NULL) {
		/* run is guaranteed to have available space. */
		arena_run_tree_remove(&bin->runs, mapelm);
		run = (arena_run_t *)(mapelm->bits & ~PAGE_MASK);
#ifdef JEMALLOC_STATS
		bin->stats.reruns++;
#endif
		return (run);
	}
	/* No existing runs have any space available. */

	/* Allocate a new run. */
	run = arena_run_alloc(arena, bin->run_size, false, false);
	if (run == NULL)
		return (NULL);

	/* Initialize run internals. */
	run->bin = bin;

	for (i = 0; i < bin->regs_mask_nelms - 1; i++)
		run->regs_mask[i] = UINT_MAX;
	remainder = bin->nregs & ((1U << (SIZEOF_INT_2POW + 3)) - 1);
	if (remainder == 0)
		run->regs_mask[i] = UINT_MAX;
	else {
		/* The last element has spare bits that need to be unset. */
		run->regs_mask[i] = (UINT_MAX >> ((1U << (SIZEOF_INT_2POW + 3))
		    - remainder));
	}

	run->regs_minelm = 0;

	run->nfree = bin->nregs;
#ifdef JEMALLOC_DEBUG
	run->magic = ARENA_RUN_MAGIC;
#endif

#ifdef JEMALLOC_STATS
	bin->stats.nruns++;
	bin->stats.curruns++;
	if (bin->stats.curruns > bin->stats.highruns)
		bin->stats.highruns = bin->stats.curruns;
#endif
	return (run);
}

/* bin->runcur must have space available before this function is called. */
static inline void *
arena_bin_malloc_easy(arena_t *arena, arena_bin_t *bin, arena_run_t *run)
{
	void *ret;

	assert(run->magic == ARENA_RUN_MAGIC);
	assert(run->nfree > 0);

	ret = arena_run_reg_alloc(run, bin);
	assert(ret != NULL);
	run->nfree--;

	return (ret);
}

/* Re-fill bin->runcur, then call arena_bin_malloc_easy(). */
static void *
arena_bin_malloc_hard(arena_t *arena, arena_bin_t *bin)
{

	bin->runcur = arena_bin_nonfull_run_get(arena, bin);
	if (bin->runcur == NULL)
		return (NULL);
	assert(bin->runcur->magic == ARENA_RUN_MAGIC);
	assert(bin->runcur->nfree > 0);

	return (arena_bin_malloc_easy(arena, bin, bin->runcur));
}

/*
 * Calculate bin->run_size such that it meets the following constraints:
 *
 *   *) bin->run_size >= min_run_size
 *   *) bin->run_size <= arena_maxclass
 *   *) bin->run_size <= RUN_MAX_SMALL
 *   *) run header overhead <= RUN_MAX_OVRHD (or header overhead relaxed).
 *
 * bin->nregs, bin->regs_mask_nelms, and bin->reg0_offset are
 * also calculated here, since these settings are all interdependent.
 */
static size_t
arena_bin_run_size_calc(arena_bin_t *bin, size_t min_run_size)
{
	size_t try_run_size, good_run_size;
	unsigned good_nregs, good_mask_nelms, good_reg0_offset;
	unsigned try_nregs, try_mask_nelms, try_reg0_offset;

	assert(min_run_size >= PAGE_SIZE);
	assert(min_run_size <= arena_maxclass);
	assert(min_run_size <= RUN_MAX_SMALL);

	/*
	 * Calculate known-valid settings before entering the run_size
	 * expansion loop, so that the first part of the loop always copies
	 * valid settings.
	 *
	 * The do..while loop iteratively reduces the number of regions until
	 * the run header and the regions no longer overlap.  A closed formula
	 * would be quite messy, since there is an interdependency between the
	 * header's mask length and the number of regions.
	 */
	try_run_size = min_run_size;
	try_nregs = ((try_run_size - sizeof(arena_run_t)) / bin->reg_size)
	    + 1; /* Counter-act try_nregs-- in loop. */
	do {
		try_nregs--;
		try_mask_nelms = (try_nregs >> (SIZEOF_INT_2POW + 3)) +
		    ((try_nregs & ((1U << (SIZEOF_INT_2POW + 3)) - 1)) ? 1 : 0);
		try_reg0_offset = try_run_size - (try_nregs * bin->reg_size);
	} while (sizeof(arena_run_t) + (sizeof(unsigned) * (try_mask_nelms - 1))
	    > try_reg0_offset);

	/* run_size expansion loop. */
	do {
		/*
		 * Copy valid settings before trying more aggressive settings.
		 */
		good_run_size = try_run_size;
		good_nregs = try_nregs;
		good_mask_nelms = try_mask_nelms;
		good_reg0_offset = try_reg0_offset;

		/* Try more aggressive settings. */
		try_run_size += PAGE_SIZE;
		try_nregs = ((try_run_size - sizeof(arena_run_t)) /
		    bin->reg_size) + 1; /* Counter-act try_nregs-- in loop. */
		do {
			try_nregs--;
			try_mask_nelms = (try_nregs >> (SIZEOF_INT_2POW + 3)) +
			    ((try_nregs & ((1U << (SIZEOF_INT_2POW + 3)) - 1)) ?
			    1 : 0);
			try_reg0_offset = try_run_size - (try_nregs *
			    bin->reg_size);
		} while (sizeof(arena_run_t) + (sizeof(unsigned) *
		    (try_mask_nelms - 1)) > try_reg0_offset);
	} while (try_run_size <= arena_maxclass && try_run_size <= RUN_MAX_SMALL
	    && RUN_MAX_OVRHD * (bin->reg_size << 3) > RUN_MAX_OVRHD_RELAX
	    && (try_reg0_offset << RUN_BFP) > RUN_MAX_OVRHD * try_run_size);

	assert(sizeof(arena_run_t) + (sizeof(unsigned) * (good_mask_nelms - 1))
	    <= good_reg0_offset);
	assert((good_mask_nelms << (SIZEOF_INT_2POW + 3)) >= good_nregs);

	/* Copy final settings. */
	bin->run_size = good_run_size;
	bin->nregs = good_nregs;
	bin->regs_mask_nelms = good_mask_nelms;
	bin->reg0_offset = good_reg0_offset;

	return (good_run_size);
}

#ifdef JEMALLOC_BALANCE
static inline void
arena_lock_balance(arena_t *arena)
{
	unsigned contention;

	contention = malloc_spin_lock(&arena->lock);
	if (narenas > 1) {
		/*
		 * Calculate the exponentially averaged contention for this
		 * arena.  Due to integer math always rounding down, this value
		 * decays somewhat faster than normal.
		 */
		arena->contention = (((uint64_t)arena->contention
		    * (uint64_t)((1U << BALANCE_ALPHA_INV_2POW)-1))
		    + (uint64_t)contention) >> BALANCE_ALPHA_INV_2POW;
		if (arena->contention >= opt_balance_threshold)
			arena_lock_balance_hard(arena);
	}
}

static void
arena_lock_balance_hard(arena_t *arena)
{
	uint32_t ind;

	arena->contention = 0;
#ifdef JEMALLOC_STATS
	arena->stats.nbalance++;
#endif
	ind = PRN(balance, narenas_2pow);
	if (arenas[ind] != NULL)
		arenas_map = arenas[ind];
	else {
		malloc_spin_lock(&arenas_lock);
		if (arenas[ind] != NULL)
			arenas_map = arenas[ind];
		else
			arenas_map = arenas_extend(ind);
		malloc_spin_unlock(&arenas_lock);
	}
}
#endif

#ifdef JEMALLOC_MAG
static inline void *
mag_alloc(mag_t *mag)
{

	if (mag->nrounds == 0)
		return (NULL);
	mag->nrounds--;

	return (mag->rounds[mag->nrounds]);
}

static void
mag_load(mag_t *mag)
{
	arena_t *arena;
	arena_bin_t *bin;
	arena_run_t *run;
	void *round;
	size_t i;

	arena = choose_arena();
	bin = &arena->bins[mag->binind];
#ifdef JEMALLOC_BALANCE
	arena_lock_balance(arena);
#else
	malloc_spin_lock(&arena->lock);
#endif
	for (i = mag->nrounds; i < max_rounds; i++) {
		if ((run = bin->runcur) != NULL && run->nfree > 0)
			round = arena_bin_malloc_easy(arena, bin, run);
		else
			round = arena_bin_malloc_hard(arena, bin);
		if (round == NULL)
			break;
		mag->rounds[i] = round;
	}
#ifdef JEMALLOC_STATS
	bin->stats.nmags++;
	arena->stats.nmalloc_small += (i - mag->nrounds);
	arena->stats.allocated_small += (i - mag->nrounds) * bin->reg_size;
#endif
	malloc_spin_unlock(&arena->lock);
	mag->nrounds = i;
}

static inline void *
mag_rack_alloc(mag_rack_t *rack, size_t size, bool zero)
{
	void *ret;
	bin_mags_t *bin_mags;
	mag_t *mag;
	size_t binind;

	binind = size2bin[size];
	assert(binind < nbins);
	bin_mags = &rack->bin_mags[binind];

	mag = bin_mags->curmag;
	if (mag == NULL) {
		/* Create an initial magazine for this size class. */
		assert(bin_mags->sparemag == NULL);
		mag = mag_create(choose_arena(), binind);
		if (mag == NULL)
			return (NULL);
		bin_mags->curmag = mag;
		mag_load(mag);
	}

	ret = mag_alloc(mag);
	if (ret == NULL) {
		if (bin_mags->sparemag != NULL) {
			if (bin_mags->sparemag->nrounds > 0) {
				/* Swap magazines. */
				bin_mags->curmag = bin_mags->sparemag;
				bin_mags->sparemag = mag;
				mag = bin_mags->curmag;
			} else {
				/* Reload the current magazine. */
				mag_load(mag);
			}
		} else {
			/* Create a second magazine. */
			mag = mag_create(choose_arena(), binind);
			if (mag == NULL)
				return (NULL);
			mag_load(mag);
			bin_mags->sparemag = bin_mags->curmag;
			bin_mags->curmag = mag;
		}
		ret = mag_alloc(mag);
		if (ret == NULL)
			return (NULL);
	}

	if (zero == false) {
#ifdef JEMALLOC_FILL
		if (opt_junk)
			memset(ret, 0xa5, size);
		else if (opt_zero)
			memset(ret, 0, size);
#endif
	} else
		memset(ret, 0, size);

	return (ret);
}
#endif

static inline void *
arena_malloc_small(arena_t *arena, size_t size, bool zero)
{
	void *ret;
	arena_bin_t *bin;
	arena_run_t *run;
	size_t binind;

	binind = size2bin[size];
	assert(binind < nbins);
	bin = &arena->bins[binind];
	size = bin->reg_size;

#ifdef JEMALLOC_BALANCE
	arena_lock_balance(arena);
#else
	malloc_spin_lock(&arena->lock);
#endif
	if ((run = bin->runcur) != NULL && run->nfree > 0)
		ret = arena_bin_malloc_easy(arena, bin, run);
	else
		ret = arena_bin_malloc_hard(arena, bin);

	if (ret == NULL) {
		malloc_spin_unlock(&arena->lock);
		return (NULL);
	}

#ifdef JEMALLOC_STATS
	bin->stats.nrequests++;
	arena->stats.nmalloc_small++;
	arena->stats.allocated_small += size;
#endif
	malloc_spin_unlock(&arena->lock);

	if (zero == false) {
#ifdef JEMALLOC_FILL
		if (opt_junk)
			memset(ret, 0xa5, size);
		else if (opt_zero)
			memset(ret, 0, size);
#endif
	} else
		memset(ret, 0, size);

	return (ret);
}

static void *
arena_malloc_large(arena_t *arena, size_t size, bool zero)
{
	void *ret;

	/* Large allocation. */
	size = PAGE_CEILING(size);
#ifdef JEMALLOC_BALANCE
	arena_lock_balance(arena);
#else
	malloc_spin_lock(&arena->lock);
#endif
	ret = (void *)arena_run_alloc(arena, size, true, zero);
	if (ret == NULL) {
		malloc_spin_unlock(&arena->lock);
		return (NULL);
	}
#ifdef JEMALLOC_STATS
	arena->stats.nmalloc_large++;
	arena->stats.allocated_large += size;
#endif
	malloc_spin_unlock(&arena->lock);

	if (zero == false) {
#ifdef JEMALLOC_FILL
		if (opt_junk)
			memset(ret, 0xa5, size);
		else if (opt_zero)
			memset(ret, 0, size);
#endif
	}

	return (ret);
}

static inline void *
arena_malloc(size_t size, bool zero)
{

	assert(size != 0);
	assert(QUANTUM_CEILING(size) <= arena_maxclass);

	if (size <= bin_maxclass) {
#ifdef JEMALLOC_MAG
		if (opt_mag) {
			mag_rack_t *rack = mag_rack;
			if (rack == NULL) {
				rack = mag_rack_create(choose_arena());
				if (rack == NULL)
					return (NULL);
				mag_rack = rack;
				pthread_setspecific(mag_rack_tsd, rack);
			}
			return (mag_rack_alloc(rack, size, zero));
		} else
#endif
			return (arena_malloc_small(choose_arena(), size, zero));
	} else
		return (arena_malloc_large(choose_arena(), size, zero));
}

static inline void *
imalloc(size_t size)
{

	assert(size != 0);

	if (size <= arena_maxclass)
		return (arena_malloc(size, false));
	else
		return (huge_malloc(size, false));
}

static inline void *
icalloc(size_t size)
{

	if (size <= arena_maxclass)
		return (arena_malloc(size, true));
	else
		return (huge_malloc(size, true));
}

/* Only handles large allocations that require more than page alignment. */
static void *
arena_palloc(arena_t *arena, size_t alignment, size_t size, size_t alloc_size)
{
	void *ret;
	size_t offset;
	arena_chunk_t *chunk;

	assert((size & PAGE_MASK) == 0);
	assert((alignment & PAGE_MASK) == 0);

#ifdef JEMALLOC_BALANCE
	arena_lock_balance(arena);
#else
	malloc_spin_lock(&arena->lock);
#endif
	ret = (void *)arena_run_alloc(arena, alloc_size, true, false);
	if (ret == NULL) {
		malloc_spin_unlock(&arena->lock);
		return (NULL);
	}

	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ret);

	offset = (uintptr_t)ret & (alignment - 1);
	assert((offset & PAGE_MASK) == 0);
	assert(offset < alloc_size);
	if (offset == 0)
		arena_run_trim_tail(arena, chunk, ret, alloc_size, size, false);
	else {
		size_t leadsize, trailsize;

		leadsize = alignment - offset;
		if (leadsize > 0) {
			arena_run_trim_head(arena, chunk, ret, alloc_size,
			    alloc_size - leadsize);
			ret = (void *)((uintptr_t)ret + leadsize);
		}

		trailsize = alloc_size - leadsize - size;
		if (trailsize != 0) {
			/* Trim trailing space. */
			assert(trailsize < alloc_size);
			arena_run_trim_tail(arena, chunk, ret, size + trailsize,
			    size, false);
		}
	}

#ifdef JEMALLOC_STATS
	arena->stats.nmalloc_large++;
	arena->stats.allocated_large += size;
#endif
	malloc_spin_unlock(&arena->lock);

#ifdef JEMALLOC_FILL
	if (opt_junk)
		memset(ret, 0xa5, size);
	else if (opt_zero)
		memset(ret, 0, size);
#endif
	return (ret);
}

static inline void *
ipalloc(size_t alignment, size_t size)
{
	void *ret;
	size_t ceil_size;

	/*
	 * Round size up to the nearest multiple of alignment.
	 *
	 * This done, we can take advantage of the fact that for each small
	 * size class, every object is aligned at the smallest power of two
	 * that is non-zero in the base two representation of the size.  For
	 * example:
	 *
	 *   Size |   Base 2 | Minimum alignment
	 *   -----+----------+------------------
	 *     96 |  1100000 |  32
	 *    144 | 10100000 |  32
	 *    192 | 11000000 |  64
	 *
	 * Depending on runtime settings, it is possible that arena_malloc()
	 * will further round up to a power of two, but that never causes
	 * correctness issues.
	 */
	ceil_size = (size + (alignment - 1)) & (-alignment);
	/*
	 * (ceil_size < size) protects against the combination of maximal
	 * alignment and size greater than maximal alignment.
	 */
	if (ceil_size < size) {
		/* size_t overflow. */
		return (NULL);
	}

	if (ceil_size <= PAGE_SIZE || (alignment <= PAGE_SIZE
	    && ceil_size <= arena_maxclass))
		ret = arena_malloc(ceil_size, false);
	else {
		size_t run_size;

		/*
		 * We can't achieve subpage alignment, so round up alignment
		 * permanently; it makes later calculations simpler.
		 */
		alignment = PAGE_CEILING(alignment);
		ceil_size = PAGE_CEILING(size);
		/*
		 * (ceil_size < size) protects against very large sizes within
		 * PAGE_SIZE of SIZE_T_MAX.
		 *
		 * (ceil_size + alignment < ceil_size) protects against the
		 * combination of maximal alignment and ceil_size large enough
		 * to cause overflow.  This is similar to the first overflow
		 * check above, but it needs to be repeated due to the new
		 * ceil_size value, which may now be *equal* to maximal
		 * alignment, whereas before we only detected overflow if the
		 * original size was *greater* than maximal alignment.
		 */
		if (ceil_size < size || ceil_size + alignment < ceil_size) {
			/* size_t overflow. */
			return (NULL);
		}

		/*
		 * Calculate the size of the over-size run that arena_palloc()
		 * would need to allocate in order to guarantee the alignment.
		 */
		if (ceil_size >= alignment)
			run_size = ceil_size + alignment - PAGE_SIZE;
		else {
			/*
			 * It is possible that (alignment << 1) will cause
			 * overflow, but it doesn't matter because we also
			 * subtract PAGE_SIZE, which in the case of overflow
			 * leaves us with a very large run_size.  That causes
			 * the first conditional below to fail, which means
			 * that the bogus run_size value never gets used for
			 * anything important.
			 */
			run_size = (alignment << 1) - PAGE_SIZE;
		}

		if (run_size <= arena_maxclass) {
			ret = arena_palloc(choose_arena(), alignment, ceil_size,
			    run_size);
		} else if (alignment <= chunksize)
			ret = huge_malloc(ceil_size, false);
		else
			ret = huge_palloc(alignment, ceil_size);
	}

	assert(((uintptr_t)ret & (alignment - 1)) == 0);
	return (ret);
}

/* Return the size of the allocation pointed to by ptr. */
static size_t
arena_salloc(const void *ptr)
{
	size_t ret;
	arena_chunk_t *chunk;
	size_t pageind, mapbits;

	assert(ptr != NULL);
	assert(CHUNK_ADDR2BASE(ptr) != ptr);

	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
	pageind = (((uintptr_t)ptr - (uintptr_t)chunk) >> PAGE_SHIFT);
	mapbits = chunk->map[pageind].bits;
	assert((mapbits & CHUNK_MAP_ALLOCATED) != 0);
	if ((mapbits & CHUNK_MAP_LARGE) == 0) {
		arena_run_t *run = (arena_run_t *)(mapbits & ~PAGE_MASK);
		assert(run->magic == ARENA_RUN_MAGIC);
		ret = run->bin->reg_size;
	} else {
		ret = mapbits & ~PAGE_MASK;
		assert(ret != 0);
	}

	return (ret);
}

static inline size_t
isalloc(const void *ptr)
{
	size_t ret;
	arena_chunk_t *chunk;

	assert(ptr != NULL);

	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
	if (chunk != ptr) {
		/* Region. */
		assert(chunk->arena->magic == ARENA_MAGIC);

		ret = arena_salloc(ptr);
	} else {
		extent_node_t *node, key;

		/* Chunk (huge allocation). */

		malloc_mutex_lock(&huge_mtx);

		/* Extract from tree of huge allocations. */
		key.addr = __DECONST(void *, ptr);
		node = extent_tree_ad_search(&huge, &key);
		assert(node != NULL);

		ret = node->size;

		malloc_mutex_unlock(&huge_mtx);
	}

	return (ret);
}

static inline void
arena_dalloc_small(arena_t *arena, arena_chunk_t *chunk, void *ptr,
    arena_chunk_map_t *mapelm)
{
	arena_run_t *run;
	arena_bin_t *bin;
	size_t size;

	run = (arena_run_t *)(mapelm->bits & ~PAGE_MASK);
	assert(run->magic == ARENA_RUN_MAGIC);
	bin = run->bin;
	size = bin->reg_size;

#ifdef JEMALLOC_FILL
	if (opt_junk)
		memset(ptr, 0x5a, size);
#endif

	arena_run_reg_dalloc(run, bin, ptr, size);
	run->nfree++;

	if (run->nfree == bin->nregs) {
		/* Deallocate run. */
		if (run == bin->runcur)
			bin->runcur = NULL;
		else if (bin->nregs != 1) {
			size_t run_pageind = (((uintptr_t)run -
			    (uintptr_t)chunk)) >> PAGE_SHIFT;
			arena_chunk_map_t *run_mapelm =
			    &chunk->map[run_pageind];
			/*
			 * This block's conditional is necessary because if the
			 * run only contains one region, then it never gets
			 * inserted into the non-full runs tree.
			 */
			arena_run_tree_remove(&bin->runs, run_mapelm);
		}
#ifdef JEMALLOC_DEBUG
		run->magic = 0;
#endif
		arena_run_dalloc(arena, run, true);
#ifdef JEMALLOC_STATS
		bin->stats.curruns--;
#endif
	} else if (run->nfree == 1 && run != bin->runcur) {
		/*
		 * Make sure that bin->runcur always refers to the lowest
		 * non-full run, if one exists.
		 */
		if (bin->runcur == NULL)
			bin->runcur = run;
		else if ((uintptr_t)run < (uintptr_t)bin->runcur) {
			/* Switch runcur. */
			if (bin->runcur->nfree > 0) {
				arena_chunk_t *runcur_chunk =
				    CHUNK_ADDR2BASE(bin->runcur);
				size_t runcur_pageind =
				    (((uintptr_t)bin->runcur -
				    (uintptr_t)runcur_chunk)) >> PAGE_SHIFT;
				arena_chunk_map_t *runcur_mapelm =
				    &runcur_chunk->map[runcur_pageind];

				/* Insert runcur. */
				arena_run_tree_insert(&bin->runs,
				    runcur_mapelm);
			}
			bin->runcur = run;
		} else {
			size_t run_pageind = (((uintptr_t)run -
			    (uintptr_t)chunk)) >> PAGE_SHIFT;
			arena_chunk_map_t *run_mapelm =
			    &chunk->map[run_pageind];

			assert(arena_run_tree_search(&bin->runs, run_mapelm) ==
			    NULL);
			arena_run_tree_insert(&bin->runs, run_mapelm);
		}
	}
#ifdef JEMALLOC_STATS
	arena->stats.allocated_small -= size;
	arena->stats.ndalloc_small++;
#endif
}

#ifdef JEMALLOC_MAG
static void
mag_unload(mag_t *mag)
{
	arena_chunk_t *chunk;
	arena_t *arena;
	void *round;
	size_t i, ndeferred, nrounds;

	for (ndeferred = mag->nrounds; ndeferred > 0;) {
		nrounds = ndeferred;
		/* Lock the arena associated with the first round. */
		chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(mag->rounds[0]);
		arena = chunk->arena;
#ifdef JEMALLOC_BALANCE
		arena_lock_balance(arena);
#else
		malloc_spin_lock(&arena->lock);
#endif
		/* Deallocate every round that belongs to the locked arena. */
		for (i = ndeferred = 0; i < nrounds; i++) {
			round = mag->rounds[i];
			chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(round);
			if (chunk->arena == arena) {
				size_t pageind = (((uintptr_t)round -
				    (uintptr_t)chunk) >> PAGE_SHIFT);
				arena_chunk_map_t *mapelm =
				    &chunk->map[pageind];
				arena_dalloc_small(arena, chunk, round, mapelm);
			} else {
				/*
				 * This round was allocated via a different
				 * arena than the one that is currently locked.
				 * Stash the round, so that it can be handled
				 * in a future pass.
				 */
				mag->rounds[ndeferred] = round;
				ndeferred++;
			}
		}
		malloc_spin_unlock(&arena->lock);
	}

	mag->nrounds = 0;
}

static inline void
mag_rack_dalloc(mag_rack_t *rack, void *ptr)
{
	arena_t *arena;
	arena_chunk_t *chunk;
	arena_run_t *run;
	arena_bin_t *bin;
	bin_mags_t *bin_mags;
	mag_t *mag;
	size_t pageind, binind;
	arena_chunk_map_t *mapelm;

	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
	arena = chunk->arena;
	pageind = (((uintptr_t)ptr - (uintptr_t)chunk) >> PAGE_SHIFT);
	mapelm = &chunk->map[pageind];
	run = (arena_run_t *)(mapelm->bits & ~PAGE_MASK);
	assert(run->magic == ARENA_RUN_MAGIC);
	bin = run->bin;
	binind = ((uintptr_t)bin - (uintptr_t)&arena->bins) /
	    sizeof(arena_bin_t);
	assert(binind < nbins);

#ifdef JEMALLOC_FILL
	if (opt_junk)
		memset(ptr, 0x5a, arena->bins[binind].reg_size);
#endif

	bin_mags = &rack->bin_mags[binind];
	mag = bin_mags->curmag;
	if (mag == NULL) {
		/* Create an initial magazine for this size class. */
		assert(bin_mags->sparemag == NULL);
		mag = mag_create(choose_arena(), binind);
		if (mag == NULL) {
			malloc_spin_lock(&arena->lock);
			arena_dalloc_small(arena, chunk, ptr, mapelm);
			malloc_spin_unlock(&arena->lock);
			return;
		}
		bin_mags->curmag = mag;
	}

	if (mag->nrounds == max_rounds) {
		if (bin_mags->sparemag != NULL) {
			if (bin_mags->sparemag->nrounds < max_rounds) {
				/* Swap magazines. */
				bin_mags->curmag = bin_mags->sparemag;
				bin_mags->sparemag = mag;
				mag = bin_mags->curmag;
			} else {
				/* Unload the current magazine. */
				mag_unload(mag);
			}
		} else {
			/* Create a second magazine. */
			mag = mag_create(choose_arena(), binind);
			if (mag == NULL) {
				mag = rack->bin_mags[binind].curmag;
				mag_unload(mag);
			} else {
				bin_mags->sparemag = bin_mags->curmag;
				bin_mags->curmag = mag;
			}
		}
		assert(mag->nrounds < max_rounds);
	}
	mag->rounds[mag->nrounds] = ptr;
	mag->nrounds++;
}
#endif

static void
arena_dalloc_large(arena_t *arena, arena_chunk_t *chunk, void *ptr)
{
	/* Large allocation. */
	malloc_spin_lock(&arena->lock);

#ifdef JEMALLOC_FILL
#ifndef JEMALLOC_STATS
	if (opt_junk)
#endif
#endif
	{
#if (defined(JEMALLOC_FILL) || defined(JEMALLOC_STATS))
		size_t pageind = ((uintptr_t)ptr - (uintptr_t)chunk) >>
		    PAGE_SHIFT;
		size_t size = chunk->map[pageind].bits & ~PAGE_MASK;
#endif

#ifdef JEMALLOC_FILL
#ifdef JEMALLOC_STATS
		if (opt_junk)
#endif
			memset(ptr, 0x5a, size);
#endif
#ifdef JEMALLOC_STATS
		arena->stats.allocated_large -= size;
#endif
	}
#ifdef JEMALLOC_STATS
	arena->stats.ndalloc_large++;
#endif

	arena_run_dalloc(arena, (arena_run_t *)ptr, true);
	malloc_spin_unlock(&arena->lock);
}

static inline void
arena_dalloc(arena_t *arena, arena_chunk_t *chunk, void *ptr)
{
	size_t pageind;
	arena_chunk_map_t *mapelm;

	assert(arena != NULL);
	assert(arena->magic == ARENA_MAGIC);
	assert(chunk->arena == arena);
	assert(ptr != NULL);
	assert(CHUNK_ADDR2BASE(ptr) != ptr);

	pageind = (((uintptr_t)ptr - (uintptr_t)chunk) >> PAGE_SHIFT);
	mapelm = &chunk->map[pageind];
	assert((mapelm->bits & CHUNK_MAP_ALLOCATED) != 0);
	if ((mapelm->bits & CHUNK_MAP_LARGE) == 0) {
		/* Small allocation. */
#ifdef JEMALLOC_MAG
		if (opt_mag) {
			mag_rack_t *rack = mag_rack;
			if (rack == NULL) {
				rack = mag_rack_create(arena);
				if (rack == NULL) {
					malloc_spin_lock(&arena->lock);
					arena_dalloc_small(arena, chunk, ptr,
					    mapelm);
					malloc_spin_unlock(&arena->lock);
				}
				mag_rack = rack;
				pthread_setspecific(mag_rack_tsd, rack);
			}
			mag_rack_dalloc(rack, ptr);
		} else {
#endif
			malloc_spin_lock(&arena->lock);
			arena_dalloc_small(arena, chunk, ptr, mapelm);
			malloc_spin_unlock(&arena->lock);
#ifdef JEMALLOC_MAG
		}
#endif
	} else
		arena_dalloc_large(arena, chunk, ptr);
}

static inline void
idalloc(void *ptr)
{
	arena_chunk_t *chunk;

	assert(ptr != NULL);

	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
	if (chunk != ptr)
		arena_dalloc(chunk->arena, chunk, ptr);
	else
		huge_dalloc(ptr);
}

static void
arena_ralloc_large_shrink(arena_t *arena, arena_chunk_t *chunk, void *ptr,
    size_t size, size_t oldsize)
{

	assert(size < oldsize);

	/*
	 * Shrink the run, and make trailing pages available for other
	 * allocations.
	 */
#ifdef JEMALLOC_BALANCE
	arena_lock_balance(arena);
#else
	malloc_spin_lock(&arena->lock);
#endif
	arena_run_trim_tail(arena, chunk, (arena_run_t *)ptr, oldsize, size,
	    true);
#ifdef JEMALLOC_STATS
	arena->stats.allocated_large -= oldsize - size;
#endif
	malloc_spin_unlock(&arena->lock);
}

static bool
arena_ralloc_large_grow(arena_t *arena, arena_chunk_t *chunk, void *ptr,
    size_t size, size_t oldsize)
{
	size_t pageind = ((uintptr_t)ptr - (uintptr_t)chunk) >> PAGE_SHIFT;
	size_t npages = oldsize >> PAGE_SHIFT;

	assert(oldsize == (chunk->map[pageind].bits & ~PAGE_MASK));

	/* Try to extend the run. */
	assert(size > oldsize);
#ifdef JEMALLOC_BALANCE
	arena_lock_balance(arena);
#else
	malloc_spin_lock(&arena->lock);
#endif
	if (pageind + npages < chunk_npages && (chunk->map[pageind+npages].bits
	    & CHUNK_MAP_ALLOCATED) == 0 && (chunk->map[pageind+npages].bits &
	    ~PAGE_MASK) >= size - oldsize) {
		/*
		 * The next run is available and sufficiently large.  Split the
		 * following run, then merge the first part with the existing
		 * allocation.
		 */
		arena_run_split(arena, (arena_run_t *)((uintptr_t)chunk +
		    ((pageind+npages) << PAGE_SHIFT)), size - oldsize, true,
		    false);

		chunk->map[pageind].bits = size | CHUNK_MAP_LARGE |
		    CHUNK_MAP_ALLOCATED;
		chunk->map[pageind+npages].bits = CHUNK_MAP_LARGE |
		    CHUNK_MAP_ALLOCATED;

#ifdef JEMALLOC_STATS
		arena->stats.allocated_large += size - oldsize;
#endif
		malloc_spin_unlock(&arena->lock);
		return (false);
	}
	malloc_spin_unlock(&arena->lock);

	return (true);
}

/*
 * Try to resize a large allocation, in order to avoid copying.  This will
 * always fail if growing an object, and the following run is already in use.
 */
static bool
arena_ralloc_large(void *ptr, size_t size, size_t oldsize)
{
	size_t psize;

	psize = PAGE_CEILING(size);
	if (psize == oldsize) {
		/* Same size class. */
#ifdef JEMALLOC_FILL
		if (opt_junk && size < oldsize) {
			memset((void *)((uintptr_t)ptr + size), 0x5a, oldsize -
			    size);
		}
#endif
		return (false);
	} else {
		arena_chunk_t *chunk;
		arena_t *arena;

		chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
		arena = chunk->arena;
		assert(arena->magic == ARENA_MAGIC);

		if (psize < oldsize) {
#ifdef JEMALLOC_FILL
			/* Fill before shrinking in order avoid a race. */
			if (opt_junk) {
				memset((void *)((uintptr_t)ptr + size), 0x5a,
				    oldsize - size);
			}
#endif
			arena_ralloc_large_shrink(arena, chunk, ptr, psize,
			    oldsize);
			return (false);
		} else {
			bool ret = arena_ralloc_large_grow(arena, chunk, ptr,
			    psize, oldsize);
#ifdef JEMALLOC_FILL
			if (ret == false && opt_zero) {
				memset((void *)((uintptr_t)ptr + oldsize), 0,
				    size - oldsize);
			}
#endif
			return (ret);
		}
	}
}

static void *
arena_ralloc(void *ptr, size_t size, size_t oldsize)
{
	void *ret;
	size_t copysize;

	/* Try to avoid moving the allocation. */
	if (size <= bin_maxclass) {
		if (oldsize <= bin_maxclass && size2bin[size] ==
		    size2bin[oldsize])
			goto IN_PLACE;
	} else {
		if (oldsize > bin_maxclass && oldsize <= arena_maxclass) {
			assert(size > bin_maxclass);
			if (arena_ralloc_large(ptr, size, oldsize) == false)
				return (ptr);
		}
	}

	/*
	 * If we get here, then size and oldsize are different enough that we
	 * need to move the object.  In that case, fall back to allocating new
	 * space and copying.
	 */
	ret = arena_malloc(size, false);
	if (ret == NULL)
		return (NULL);

	/* Junk/zero-filling were already done by arena_malloc(). */
	copysize = (size < oldsize) ? size : oldsize;
	memcpy(ret, ptr, copysize);
	idalloc(ptr);
	return (ret);
IN_PLACE:
#ifdef JEMALLOC_FILL
	if (opt_junk && size < oldsize)
		memset((void *)((uintptr_t)ptr + size), 0x5a, oldsize - size);
	else if (opt_zero && size > oldsize)
		memset((void *)((uintptr_t)ptr + oldsize), 0, size - oldsize);
#endif
	return (ptr);
}

static inline void *
iralloc(void *ptr, size_t size)
{
	size_t oldsize;

	assert(ptr != NULL);
	assert(size != 0);

	oldsize = isalloc(ptr);

	if (size <= arena_maxclass)
		return (arena_ralloc(ptr, size, oldsize));
	else
		return (huge_ralloc(ptr, size, oldsize));
}

static bool
arena_new(arena_t *arena)
{
	unsigned i;
	arena_bin_t *bin;
	size_t prev_run_size;

	if (malloc_spin_init(&arena->lock))
		return (true);

#ifdef JEMALLOC_STATS
	memset(&arena->stats, 0, sizeof(arena_stats_t));
#endif

	/* Initialize chunks. */
	arena_chunk_tree_dirty_new(&arena->chunks_dirty);
	arena->spare = NULL;

	arena->ndirty = 0;

	arena_avail_tree_new(&arena->runs_avail);

#ifdef JEMALLOC_BALANCE
	arena->contention = 0;
#endif

	/* Initialize bins. */
	prev_run_size = PAGE_SIZE;

	i = 0;
#ifdef JEMALLOC_TINY
	/* (2^n)-spaced tiny bins. */
	for (; i < ntbins; i++) {
		bin = &arena->bins[i];
		bin->runcur = NULL;
		arena_run_tree_new(&bin->runs);

		bin->reg_size = (1U << (TINY_MIN_2POW + i));

		prev_run_size = arena_bin_run_size_calc(bin, prev_run_size);

#ifdef JEMALLOC_STATS
		memset(&bin->stats, 0, sizeof(malloc_bin_stats_t));
#endif
	}
#endif

	/* Quantum-spaced bins. */
	for (; i < ntbins + nqbins; i++) {
		bin = &arena->bins[i];
		bin->runcur = NULL;
		arena_run_tree_new(&bin->runs);

		bin->reg_size = (i - ntbins + 1) << QUANTUM_2POW;

		prev_run_size = arena_bin_run_size_calc(bin, prev_run_size);

#ifdef JEMALLOC_STATS
		memset(&bin->stats, 0, sizeof(malloc_bin_stats_t));
#endif
	}

	/* Cacheline-spaced bins. */
	for (; i < ntbins + nqbins + ncbins; i++) {
		bin = &arena->bins[i];
		bin->runcur = NULL;
		arena_run_tree_new(&bin->runs);

		bin->reg_size = cspace_min + ((i - (ntbins + nqbins)) <<
		    CACHELINE_2POW);

		prev_run_size = arena_bin_run_size_calc(bin, prev_run_size);

#ifdef JEMALLOC_STATS
		memset(&bin->stats, 0, sizeof(malloc_bin_stats_t));
#endif
	}

	/* Subpage-spaced bins. */
	for (; i < nbins; i++) {
		bin = &arena->bins[i];
		bin->runcur = NULL;
		arena_run_tree_new(&bin->runs);

		bin->reg_size = sspace_min + ((i - (ntbins + nqbins + ncbins))
		    << SUBPAGE_2POW);

		prev_run_size = arena_bin_run_size_calc(bin, prev_run_size);

#ifdef JEMALLOC_STATS
		memset(&bin->stats, 0, sizeof(malloc_bin_stats_t));
#endif
	}

#ifdef JEMALLOC_DEBUG
	arena->magic = ARENA_MAGIC;
#endif

	return (false);
}

/* Create a new arena and insert it into the arenas array at index ind. */
static arena_t *
arenas_extend(unsigned ind)
{
	arena_t *ret;

	/* Allocate enough space for trailing bins. */
	ret = (arena_t *)base_alloc(sizeof(arena_t)
	    + (sizeof(arena_bin_t) * (nbins - 1)));
	if (ret != NULL && arena_new(ret) == false) {
		arenas[ind] = ret;
		return (ret);
	}
	/* Only reached if there is an OOM error. */

	/*
	 * OOM here is quite inconvenient to propagate, since dealing with it
	 * would require a check for failure in the fast path.  Instead, punt
	 * by using arenas[0].  In practice, this is an extremely unlikely
	 * failure.
	 */
	jemalloc_message("<jemalloc>",
	    ": Error initializing arena\n", "", "");
	if (opt_abort)
		abort();

	return (arenas[0]);
}

#ifdef JEMALLOC_MAG
static mag_t *
mag_create(arena_t *arena, size_t binind)
{
	mag_t *ret;

	if (sizeof(mag_t) + (sizeof(void *) * (max_rounds - 1)) <=
	    bin_maxclass) {
		ret = arena_malloc_small(arena, sizeof(mag_t) + (sizeof(void *)
		    * (max_rounds - 1)), false);
	} else {
		ret = imalloc(sizeof(mag_t) + (sizeof(void *) * (max_rounds -
		    1)));
	}
	if (ret == NULL)
		return (NULL);
	ret->binind = binind;
	ret->nrounds = 0;

	return (ret);
}

static void
mag_destroy(mag_t *mag)
{
	arena_t *arena;
	arena_chunk_t *chunk;
	size_t pageind;
	arena_chunk_map_t *mapelm;

	chunk = CHUNK_ADDR2BASE(mag);
	arena = chunk->arena;
	pageind = (((uintptr_t)mag - (uintptr_t)chunk) >> PAGE_SHIFT);
	mapelm = &chunk->map[pageind];

	assert(mag->nrounds == 0);
	if (sizeof(mag_t) + (sizeof(void *) * (max_rounds - 1)) <=
	    bin_maxclass) {
		malloc_spin_lock(&arena->lock);
		arena_dalloc_small(arena, chunk, mag, mapelm);
		malloc_spin_unlock(&arena->lock);
	} else
		idalloc(mag);
}

static mag_rack_t *
mag_rack_create(arena_t *arena)
{

	assert(sizeof(mag_rack_t) + (sizeof(bin_mags_t *) * (nbins - 1)) <=
	    bin_maxclass);
	return (arena_malloc_small(arena, sizeof(mag_rack_t) +
	    (sizeof(bin_mags_t) * (nbins - 1)), true));
}

static void
mag_rack_destroy(mag_rack_t *rack)
{
	arena_t *arena;
	arena_chunk_t *chunk;
	bin_mags_t *bin_mags;
	size_t i, pageind;
	arena_chunk_map_t *mapelm;

	for (i = 0; i < nbins; i++) {
		bin_mags = &rack->bin_mags[i];
		if (bin_mags->curmag != NULL) {
			assert(bin_mags->curmag->binind == i);
			mag_unload(bin_mags->curmag);
			mag_destroy(bin_mags->curmag);
		}
		if (bin_mags->sparemag != NULL) {
			assert(bin_mags->sparemag->binind == i);
			mag_unload(bin_mags->sparemag);
			mag_destroy(bin_mags->sparemag);
		}
	}

	chunk = CHUNK_ADDR2BASE(rack);
	arena = chunk->arena;
	pageind = (((uintptr_t)rack - (uintptr_t)chunk) >> PAGE_SHIFT);
	mapelm = &chunk->map[pageind];

	malloc_spin_lock(&arena->lock);
	arena_dalloc_small(arena, chunk, rack, mapelm);
	malloc_spin_unlock(&arena->lock);
}
#endif

/*
 * End arena.
 */
/******************************************************************************/
/*
 * Begin general internal functions.
 */

static void *
huge_malloc(size_t size, bool zero)
{
	void *ret;
	size_t csize;
	extent_node_t *node;

	/* Allocate one or more contiguous chunks for this request. */

	csize = CHUNK_CEILING(size);
	if (csize == 0) {
		/* size is large enough to cause size_t wrap-around. */
		return (NULL);
	}

	/* Allocate an extent node with which to track the chunk. */
	node = base_node_alloc();
	if (node == NULL)
		return (NULL);

	ret = chunk_alloc(csize, zero);
	if (ret == NULL) {
		base_node_dealloc(node);
		return (NULL);
	}

	/* Insert node into huge. */
	node->addr = ret;
	node->size = csize;

	malloc_mutex_lock(&huge_mtx);
	extent_tree_ad_insert(&huge, node);
#ifdef JEMALLOC_STATS
	huge_nmalloc++;
	huge_allocated += csize;
#endif
	malloc_mutex_unlock(&huge_mtx);

#ifdef JEMALLOC_FILL
	if (zero == false) {
		if (opt_junk)
			memset(ret, 0xa5, csize);
		else if (opt_zero)
			memset(ret, 0, csize);
	}
#endif

	return (ret);
}

/* Only handles large allocations that require more than chunk alignment. */
static void *
huge_palloc(size_t alignment, size_t size)
{
	void *ret;
	size_t alloc_size, chunk_size, offset;
	extent_node_t *node;

	/*
	 * This allocation requires alignment that is even larger than chunk
	 * alignment.  This means that huge_malloc() isn't good enough.
	 *
	 * Allocate almost twice as many chunks as are demanded by the size or
	 * alignment, in order to assure the alignment can be achieved, then
	 * unmap leading and trailing chunks.
	 */
	assert(alignment >= chunksize);

	chunk_size = CHUNK_CEILING(size);

	if (size >= alignment)
		alloc_size = chunk_size + alignment - chunksize;
	else
		alloc_size = (alignment << 1) - chunksize;

	/* Allocate an extent node with which to track the chunk. */
	node = base_node_alloc();
	if (node == NULL)
		return (NULL);

	ret = chunk_alloc(alloc_size, false);
	if (ret == NULL) {
		base_node_dealloc(node);
		return (NULL);
	}

	offset = (uintptr_t)ret & (alignment - 1);
	assert((offset & chunksize_mask) == 0);
	assert(offset < alloc_size);
	if (offset == 0) {
		/* Trim trailing space. */
		chunk_dealloc((void *)((uintptr_t)ret + chunk_size), alloc_size
		    - chunk_size);
	} else {
		size_t trailsize;

		/* Trim leading space. */
		chunk_dealloc(ret, alignment - offset);

		ret = (void *)((uintptr_t)ret + (alignment - offset));

		trailsize = alloc_size - (alignment - offset) - chunk_size;
		if (trailsize != 0) {
		    /* Trim trailing space. */
		    assert(trailsize < alloc_size);
		    chunk_dealloc((void *)((uintptr_t)ret + chunk_size),
			trailsize);
		}
	}

	/* Insert node into huge. */
	node->addr = ret;
	node->size = chunk_size;

	malloc_mutex_lock(&huge_mtx);
	extent_tree_ad_insert(&huge, node);
#ifdef JEMALLOC_STATS
	huge_nmalloc++;
	huge_allocated += chunk_size;
#endif
	malloc_mutex_unlock(&huge_mtx);

#ifdef JEMALLOC_FILL
	if (opt_junk)
		memset(ret, 0xa5, chunk_size);
	else if (opt_zero)
		memset(ret, 0, chunk_size);
#endif

	return (ret);
}

static void *
huge_ralloc(void *ptr, size_t size, size_t oldsize)
{
	void *ret;
	size_t copysize;

	/* Avoid moving the allocation if the size class would not change. */
	if (oldsize > arena_maxclass &&
	    CHUNK_CEILING(size) == CHUNK_CEILING(oldsize)) {
#ifdef JEMALLOC_FILL
		if (opt_junk && size < oldsize) {
			memset((void *)((uintptr_t)ptr + size), 0x5a, oldsize
			    - size);
		} else if (opt_zero && size > oldsize) {
			memset((void *)((uintptr_t)ptr + oldsize), 0, size
			    - oldsize);
		}
#endif
		return (ptr);
	}

	/*
	 * If we get here, then size and oldsize are different enough that we
	 * need to use a different size class.  In that case, fall back to
	 * allocating new space and copying.
	 */
	ret = huge_malloc(size, false);
	if (ret == NULL)
		return (NULL);

	copysize = (size < oldsize) ? size : oldsize;
	memcpy(ret, ptr, copysize);
	idalloc(ptr);
	return (ret);
}

static void
huge_dalloc(void *ptr)
{
	extent_node_t *node, key;

	malloc_mutex_lock(&huge_mtx);

	/* Extract from tree of huge allocations. */
	key.addr = ptr;
	node = extent_tree_ad_search(&huge, &key);
	assert(node != NULL);
	assert(node->addr == ptr);
	extent_tree_ad_remove(&huge, node);

#ifdef JEMALLOC_STATS
	huge_ndalloc++;
	huge_allocated -= node->size;
#endif

	malloc_mutex_unlock(&huge_mtx);

	/* Unmap chunk. */
#ifdef JEMALLOC_FILL
#ifdef JEMALLOC_DSS
	if (opt_dss && opt_junk)
		memset(node->addr, 0x5a, node->size);
#endif
#endif
	chunk_dealloc(node->addr, node->size);

	base_node_dealloc(node);
}

static void
malloc_print_stats(void)
{

	if (opt_print_stats) {
		char s[UMAX2S_BUFSIZE];
		jemalloc_message("___ Begin jemalloc statistics ___\n", "", "",
		    "");
		jemalloc_message("Assertions ",
#ifdef NDEBUG
		    "disabled",
#else
		    "enabled",
#endif
		    "\n", "");
		jemalloc_message("Boolean JEMALLOC_OPTIONS: ",
		    opt_abort ? "A" : "a", "", "");
#ifdef JEMALLOC_DSS
		jemalloc_message(opt_dss ? "D" : "d", "", "", "");
#endif
#ifdef JEMALLOC_MAG
		jemalloc_message(opt_mag ? "G" : "g", "", "", "");
#endif
#ifdef JEMALLOC_FILL
		jemalloc_message(opt_junk ? "J" : "j", "", "", "");
#endif
#ifdef JEMALLOC_DSS
		jemalloc_message(opt_mmap ? "M" : "m", "", "", "");
#endif
		jemalloc_message("P", "", "", "");
#ifdef JEMALLOC_STATS
		jemalloc_message(opt_utrace ? "U" : "u", "", "", "");
#endif
#ifdef JEMALLOC_SYSV
		jemalloc_message(opt_sysv ? "V" : "v", "", "", "");
#endif
#ifdef JEMALLOC_XMALLOC
		jemalloc_message(opt_xmalloc ? "X" : "x", "", "", "");
#endif
#ifdef JEMALLOC_FILL
		jemalloc_message(opt_zero ? "Z" : "z", "", "", "");
#endif
		jemalloc_message("\n", "", "", "");

		jemalloc_message("CPUs: ", umax2s(ncpus, s), "\n", "");
		jemalloc_message("Max arenas: ", umax2s(narenas, s), "\n", "");
#ifdef JEMALLOC_BALANCE
		jemalloc_message("Arena balance threshold: ",
		    umax2s(opt_balance_threshold, s), "\n", "");
#endif
		jemalloc_message("Pointer size: ", umax2s(sizeof(void *), s),
		    "\n", "");
		jemalloc_message("Quantum size: ", umax2s(QUANTUM, s), "\n",
		    "");
		jemalloc_message("Cacheline size (assumed): ",
		    umax2s(CACHELINE, s), "\n", "");
#ifdef JEMALLOC_TINY
		jemalloc_message("Tiny 2^n-spaced sizes: [", umax2s((1U <<
		    TINY_MIN_2POW), s), "..", "");
		jemalloc_message(umax2s((qspace_min >> 1), s), "]\n", "", "");
#endif
		jemalloc_message("Quantum-spaced sizes: [", umax2s(qspace_min,
		    s), "..", "");
		jemalloc_message(umax2s(qspace_max, s), "]\n", "", "");
		jemalloc_message("Cacheline-spaced sizes: [",
		    umax2s(cspace_min, s), "..", "");
		jemalloc_message(umax2s(cspace_max, s), "]\n", "", "");
		jemalloc_message("Subpage-spaced sizes: [", umax2s(sspace_min,
		    s), "..", "");
		jemalloc_message(umax2s(sspace_max, s), "]\n", "", "");
#ifdef JEMALLOC_MAG
		jemalloc_message("Rounds per magazine: ", umax2s(max_rounds,
		    s), "\n", "");
#endif
		jemalloc_message("Max dirty pages per arena: ",
		    umax2s(opt_dirty_max, s), "\n", "");

		jemalloc_message("Chunk size: ", umax2s(chunksize, s), "", "");
		jemalloc_message(" (2^", umax2s(opt_chunk_2pow, s), ")\n", "");

#ifdef JEMALLOC_STATS
		{
			size_t allocated, mapped;
#ifdef JEMALLOC_BALANCE
			uint64_t nbalance = 0;
#endif
			unsigned i;
			arena_t *arena;

			/* Calculate and print allocated/mapped stats. */

			/* arenas. */
			for (i = 0, allocated = 0; i < narenas; i++) {
				if (arenas[i] != NULL) {
					malloc_spin_lock(&arenas[i]->lock);
					allocated +=
					    arenas[i]->stats.allocated_small;
					allocated +=
					    arenas[i]->stats.allocated_large;
#ifdef JEMALLOC_BALANCE
					nbalance += arenas[i]->stats.nbalance;
#endif
					malloc_spin_unlock(&arenas[i]->lock);
				}
			}

			/* huge/base. */
			malloc_mutex_lock(&huge_mtx);
			allocated += huge_allocated;
			mapped = stats_chunks.curchunks * chunksize;
			malloc_mutex_unlock(&huge_mtx);

			malloc_mutex_lock(&base_mtx);
			mapped += base_mapped;
			malloc_mutex_unlock(&base_mtx);

			malloc_printf("Allocated: %zu, mapped: %zu\n",
			    allocated, mapped);

#ifdef JEMALLOC_BALANCE
			malloc_printf("Arena balance reassignments: %llu\n",
			    nbalance);
#endif

			/* Print chunk stats. */
			{
				chunk_stats_t chunks_stats;

				malloc_mutex_lock(&huge_mtx);
				chunks_stats = stats_chunks;
				malloc_mutex_unlock(&huge_mtx);

				malloc_printf("chunks: nchunks   "
				    "highchunks    curchunks\n");
				malloc_printf("  %13llu%13lu%13lu\n",
				    chunks_stats.nchunks,
				    chunks_stats.highchunks,
				    chunks_stats.curchunks);
			}

			/* Print chunk stats. */
			malloc_printf(
			    "huge: nmalloc      ndalloc    allocated\n");
			malloc_printf(" %12llu %12llu %12zu\n",
			    huge_nmalloc, huge_ndalloc, huge_allocated);

			/* Print stats for each arena. */
			for (i = 0; i < narenas; i++) {
				arena = arenas[i];
				if (arena != NULL) {
					malloc_printf(
					    "\narenas[%u]:\n", i);
					malloc_spin_lock(&arena->lock);
					stats_print(arena);
					malloc_spin_unlock(&arena->lock);
				}
			}
		}
#endif /* #ifdef JEMALLOC_STATS */
		jemalloc_message("--- End jemalloc statistics ---\n", "", "",
		    "");
	}
}

#ifdef JEMALLOC_DEBUG
static void
size2bin_validate(void)
{
	size_t i, size, binind;

	assert(size2bin[0] == 0xffU);
	i = 1;
#  ifdef JEMALLOC_TINY
	/* Tiny. */
	for (; i < (1U << TINY_MIN_2POW); i++) {
		size = pow2_ceil(1U << TINY_MIN_2POW);
		binind = ffs((int)(size >> (TINY_MIN_2POW + 1)));
		assert(size2bin[i] == binind);
	}
	for (; i < qspace_min; i++) {
		size = pow2_ceil(i);
		binind = ffs((int)(size >> (TINY_MIN_2POW + 1)));
		assert(size2bin[i] == binind);
	}
#  endif
	/* Quantum-spaced. */
	for (; i <= qspace_max; i++) {
		size = QUANTUM_CEILING(i);
		binind = ntbins + (size >> QUANTUM_2POW) - 1;
		assert(size2bin[i] == binind);
	}
	/* Cacheline-spaced. */
	for (; i <= cspace_max; i++) {
		size = CACHELINE_CEILING(i);
		binind = ntbins + nqbins + ((size - cspace_min) >>
		    CACHELINE_2POW);
		assert(size2bin[i] == binind);
	}
	/* Sub-page. */
	for (; i <= sspace_max; i++) {
		size = SUBPAGE_CEILING(i);
		binind = ntbins + nqbins + ncbins + ((size - sspace_min)
		    >> SUBPAGE_2POW);
		assert(size2bin[i] == binind);
	}
}
#endif

static bool
size2bin_init(void)
{

	if (opt_qspace_max_2pow != QSPACE_MAX_2POW_DEFAULT
	    || opt_cspace_max_2pow != CSPACE_MAX_2POW_DEFAULT)
		return (size2bin_init_hard());

	size2bin = const_size2bin;
#ifdef JEMALLOC_DEBUG
	assert(sizeof(const_size2bin) == bin_maxclass + 1);
	size2bin_validate();
#endif
	return (false);
}

static bool
size2bin_init_hard(void)
{
	size_t i, size, binind;
	uint8_t *custom_size2bin;

	assert(opt_qspace_max_2pow != QSPACE_MAX_2POW_DEFAULT
	    || opt_cspace_max_2pow != CSPACE_MAX_2POW_DEFAULT);

	custom_size2bin = (uint8_t *)base_alloc(bin_maxclass + 1);
	if (custom_size2bin == NULL)
		return (true);

	custom_size2bin[0] = 0xffU;
	i = 1;
#ifdef JEMALLOC_TINY
	/* Tiny. */
	for (; i < (1U << TINY_MIN_2POW); i++) {
		size = pow2_ceil(1U << TINY_MIN_2POW);
		binind = ffs((int)(size >> (TINY_MIN_2POW + 1)));
		custom_size2bin[i] = binind;
	}
	for (; i < qspace_min; i++) {
		size = pow2_ceil(i);
		binind = ffs((int)(size >> (TINY_MIN_2POW + 1)));
		custom_size2bin[i] = binind;
	}
#endif
	/* Quantum-spaced. */
	for (; i <= qspace_max; i++) {
		size = QUANTUM_CEILING(i);
		binind = ntbins + (size >> QUANTUM_2POW) - 1;
		custom_size2bin[i] = binind;
	}
	/* Cacheline-spaced. */
	for (; i <= cspace_max; i++) {
		size = CACHELINE_CEILING(i);
		binind = ntbins + nqbins + ((size - cspace_min) >>
		    CACHELINE_2POW);
		custom_size2bin[i] = binind;
	}
	/* Sub-page. */
	for (; i <= sspace_max; i++) {
		size = SUBPAGE_CEILING(i);
		binind = ntbins + nqbins + ncbins + ((size - sspace_min) >>
		    SUBPAGE_2POW);
		custom_size2bin[i] = binind;
	}

	size2bin = custom_size2bin;
#ifdef JEMALLOC_DEBUG
	size2bin_validate();
#endif
	return (false);
}

static unsigned
malloc_ncpus(void)
{
	unsigned ret;
	long result;

	result = sysconf(_SC_NPROCESSORS_ONLN);
	if (result == -1) {
		/* Error. */
		ret = 1;
	}
	ret = (unsigned)result;

	return (ret);
}

/*
 * FreeBSD's pthreads implementation calls malloc(3), so the malloc
 * implementation has to take pains to avoid infinite recursion during
 * initialization.
 */
static inline bool
malloc_init(void)
{

	if (malloc_initialized == false)
		return (malloc_init_hard());

	return (false);
}

static bool
malloc_init_hard(void)
{
	unsigned i;
	int linklen;
	char buf[PATH_MAX + 1];
	const char *opts;
	arena_t *init_arenas[1];

	malloc_mutex_lock(&init_lock);
	if (malloc_initialized || malloc_initializer == pthread_self()) {
		/*
		 * Another thread initialized the allocator before this one
		 * acquired init_lock, or this thread is the inializing thread,
		 * and it is recursively allocating.
		 */
		malloc_mutex_unlock(&init_lock);
		return (false);
	}
	if (malloc_initializer != (unsigned long)0) {
		/* Busy-wait until the initializing thread completes. */
		do {
			malloc_mutex_unlock(&init_lock);
			CPU_SPINWAIT;
			malloc_mutex_lock(&init_lock);
		} while (malloc_initialized == false);
		return (false);
	}

#ifdef DYNAMIC_PAGE_SHIFT
	/* Get page size. */
	{
		long result;

		result = sysconf(_SC_PAGESIZE);
		assert(result != -1);
		pagesize = (unsigned)result;

		/*
		 * We assume that pagesize is a power of 2 when calculating
		 * pagesize_mask and pagesize_2pow.
		 */
		assert(((result - 1) & result) == 0);
		pagesize_mask = result - 1;
		pagesize_2pow = ffs((int)result) - 1;
	}
#endif

	for (i = 0; i < 3; i++) {
		unsigned j;

		/* Get runtime configuration. */
		switch (i) {
		case 0:
			if ((linklen = readlink("/etc/jemalloc.conf", buf,
						sizeof(buf) - 1)) != -1) {
				/*
				 * Use the contents of the "/etc/jemalloc.conf"
				 * symbolic link's name.
				 */
				buf[linklen] = '\0';
				opts = buf;
			} else {
				/* No configuration specified. */
				buf[0] = '\0';
				opts = buf;
			}
			break;
		case 1:
			if ((opts = getenv("JEMALLOC_OPTIONS")) != NULL) {
				/*
				 * Do nothing; opts is already initialized to
				 * the value of the JEMALLOC_OPTIONS
				 * environment variable.
				 */
			} else {
				/* No configuration specified. */
				buf[0] = '\0';
				opts = buf;
			}
			break;
		case 2:
			if (jemalloc_options != NULL) {
				/*
				 * Use options that were compiled into the
				 * program.
				 */
				opts = jemalloc_options;
			} else {
				/* No configuration specified. */
				buf[0] = '\0';
				opts = buf;
			}
			break;
		default:
			/* NOTREACHED */
			assert(false);
			buf[0] = '\0';
			opts = buf;
		}

		for (j = 0; opts[j] != '\0'; j++) {
			unsigned k, nreps;
			bool nseen;

			/* Parse repetition count, if any. */
			for (nreps = 0, nseen = false;; j++, nseen = true) {
				switch (opts[j]) {
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9':
						nreps *= 10;
						nreps += opts[j] - '0';
						break;
					default:
						goto MALLOC_OUT;
				}
			}
MALLOC_OUT:
			if (nseen == false)
				nreps = 1;

			for (k = 0; k < nreps; k++) {
				switch (opts[j]) {
				case 'a':
					opt_abort = false;
					break;
				case 'A':
					opt_abort = true;
					break;
				case 'b':
#ifdef JEMALLOC_BALANCE
					opt_balance_threshold >>= 1;
#endif
					break;
				case 'B':
#ifdef JEMALLOC_BALANCE
					if (opt_balance_threshold == 0)
						opt_balance_threshold = 1;
					else if ((opt_balance_threshold << 1)
					    > opt_balance_threshold)
						opt_balance_threshold <<= 1;
#endif
					break;
				case 'c':
					if (opt_cspace_max_2pow - 1 >
					    opt_qspace_max_2pow &&
					    opt_cspace_max_2pow >
					    CACHELINE_2POW)
						opt_cspace_max_2pow--;
					break;
				case 'C':
					if (opt_cspace_max_2pow < PAGE_SHIFT
					    - 1)
						opt_cspace_max_2pow++;
					break;
				case 'd':
#ifdef JEMALLOC_DSS
					opt_dss = false;
#endif
					break;
				case 'D':
#ifdef JEMALLOC_DSS
					opt_dss = true;
#endif
					break;
				case 'f':
					opt_dirty_max >>= 1;
					break;
				case 'F':
					if (opt_dirty_max == 0)
						opt_dirty_max = 1;
					else if ((opt_dirty_max << 1) != 0)
						opt_dirty_max <<= 1;
					break;
#ifdef JEMALLOC_MAG
				case 'g':
					opt_mag = false;
					break;
				case 'G':
					opt_mag = true;
					break;
#endif
#ifdef JEMALLOC_FILL
				case 'j':
					opt_junk = false;
					break;
				case 'J':
					opt_junk = true;
					break;
#endif
				case 'k':
					/*
					 * Chunks always require at least one
					 * header page, so chunks can never be
					 * smaller than two pages.
					 */
					if (opt_chunk_2pow > PAGE_SHIFT + 1)
						opt_chunk_2pow--;
					break;
				case 'K':
					if (opt_chunk_2pow + 1 <
					    (sizeof(size_t) << 3))
						opt_chunk_2pow++;
					break;
				case 'm':
#ifdef JEMALLOC_DSS
					opt_mmap = false;
#endif
					break;
				case 'M':
#ifdef JEMALLOC_DSS
					opt_mmap = true;
#endif
					break;
				case 'n':
					opt_narenas_lshift--;
					break;
				case 'N':
					opt_narenas_lshift++;
					break;
				case 'p':
					opt_print_stats = false;
					break;
				case 'P':
					opt_print_stats = true;
					break;
				case 'q':
					if (opt_qspace_max_2pow > QUANTUM_2POW)
						opt_qspace_max_2pow--;
					break;
				case 'Q':
					if (opt_qspace_max_2pow + 1 <
					    opt_cspace_max_2pow)
						opt_qspace_max_2pow++;
					break;
#ifdef JEMALLOC_MAG
				case 'R':
					if (opt_mag_size_2pow + 1 < (8U <<
					    SIZEOF_PTR_2POW))
						opt_mag_size_2pow++;
					break;
				case 'r':
					/*
					 * Make sure there's always at least
					 * one round per magazine.
					 */
					if ((1U << (opt_mag_size_2pow-1)) >=
					    sizeof(mag_t))
						opt_mag_size_2pow--;
					break;
#endif
#ifdef JEMALLOC_STATS
				case 'u':
					opt_utrace = false;
					break;
				case 'U':
					opt_utrace = true;
					break;
#endif
#ifdef JEMALLOC_SYSV
				case 'v':
					opt_sysv = false;
					break;
				case 'V':
					opt_sysv = true;
					break;
#endif
#ifdef JEMALLOC_XMALLOC
				case 'x':
					opt_xmalloc = false;
					break;
				case 'X':
					opt_xmalloc = true;
					break;
#endif
#ifdef JEMALLOC_FILL
				case 'z':
					opt_zero = false;
					break;
				case 'Z':
					opt_zero = true;
					break;
#endif
				default: {
					char cbuf[2];

					cbuf[0] = opts[j];
					cbuf[1] = '\0';
					jemalloc_message("<jemalloc>",
					    ": Unsupported character "
					    "in malloc options: '", cbuf,
					    "'\n");
				}
				}
			}
		}
	}

#ifdef JEMALLOC_DSS
	/* Make sure that there is some method for acquiring memory. */
	if (opt_dss == false && opt_mmap == false)
		opt_mmap = true;
#endif

	/* Take care to call atexit() only once. */
	if (opt_print_stats) {
		/* Print statistics at exit. */
		atexit(malloc_print_stats);
	}

	/* Register fork handlers. */
	pthread_atfork(jemalloc_prefork, jemalloc_postfork, jemalloc_postfork);

#ifdef JEMALLOC_MAG
	/*
	 * Calculate the actual number of rounds per magazine, taking into
	 * account header overhead.
	 */
	max_rounds = (1LLU << (opt_mag_size_2pow - SIZEOF_PTR_2POW)) -
	    (sizeof(mag_t) >> SIZEOF_PTR_2POW) + 1;
#endif

	/* Set variables according to the value of opt_[qc]space_max_2pow. */
	qspace_max = (1U << opt_qspace_max_2pow);
	cspace_min = CACHELINE_CEILING(qspace_max);
	if (cspace_min == qspace_max)
		cspace_min += CACHELINE;
	cspace_max = (1U << opt_cspace_max_2pow);
	sspace_min = SUBPAGE_CEILING(cspace_max);
	if (sspace_min == cspace_max)
		sspace_min += SUBPAGE;
	assert(sspace_min < PAGE_SIZE);
	sspace_max = PAGE_SIZE - SUBPAGE;

#ifdef JEMALLOC_TINY
	assert(QUANTUM_2POW >= TINY_MIN_2POW);
#endif
	assert(ntbins <= QUANTUM_2POW);
	nqbins = qspace_max >> QUANTUM_2POW;
	ncbins = ((cspace_max - cspace_min) >> CACHELINE_2POW) + 1;
	nsbins = ((sspace_max - sspace_min) >> SUBPAGE_2POW) + 1;
	nbins = ntbins + nqbins + ncbins + nsbins;

	if (size2bin_init()) {
		malloc_mutex_unlock(&init_lock);
		return (true);
	}

	/* Set variables according to the value of opt_chunk_2pow. */
	chunksize = (1LU << opt_chunk_2pow);
	chunksize_mask = chunksize - 1;
	chunk_npages = (chunksize >> PAGE_SHIFT);
	{
		size_t header_size;

		/*
		 * Compute the header size such that it is large enough to
		 * contain the page map.
		 */
		header_size = sizeof(arena_chunk_t) +
		    (sizeof(arena_chunk_map_t) * (chunk_npages - 1));
		arena_chunk_header_npages = (header_size >> PAGE_SHIFT) +
		    ((header_size & PAGE_MASK) != 0);
	}
	arena_maxclass = chunksize - (arena_chunk_header_npages <<
	    PAGE_SHIFT);

	UTRACE(0, 0, 0);

#ifdef JEMALLOC_STATS
	memset(&stats_chunks, 0, sizeof(chunk_stats_t));
#endif

	/* Various sanity checks that regard configuration. */
	assert(chunksize >= PAGE_SIZE);

	/* Initialize chunks data. */
	if (malloc_mutex_init(&huge_mtx)) {
		malloc_mutex_unlock(&init_lock);
		return (true);
	}
	extent_tree_ad_new(&huge);
#ifdef JEMALLOC_DSS
	if (malloc_mutex_init(&dss_mtx)) {
		malloc_mutex_unlock(&init_lock);
		return (true);
	}
	dss_base = sbrk(0);
	dss_prev = dss_base;
	dss_max = dss_base;
	extent_tree_szad_new(&dss_chunks_szad);
	extent_tree_ad_new(&dss_chunks_ad);
#endif
#ifdef JEMALLOC_STATS
	huge_nmalloc = 0;
	huge_ndalloc = 0;
	huge_allocated = 0;
#endif

	/* Initialize base allocation data structures. */
#ifdef JEMALLOC_STATS
	base_mapped = 0;
#endif
#ifdef JEMALLOC_DSS
	/*
	 * Allocate a base chunk here, since it doesn't actually have to be
	 * chunk-aligned.  Doing this before allocating any other chunks allows
	 * the use of space that would otherwise be wasted.
	 */
	if (opt_dss)
		base_pages_alloc(0);
#endif
	base_nodes = NULL;
	if (malloc_mutex_init(&base_mtx)) {
		malloc_mutex_unlock(&init_lock);
		return (true);
	}

	/*
	 * Create enough scaffolding to allow recursive allocation in
	 * malloc_ncpus().
	 */
	narenas = 1;
	arenas = init_arenas;
	memset(arenas, 0, sizeof(arena_t *) * narenas);

	/*
	 * Initialize one arena here.  The rest are lazily created in
	 * choose_arena_hard().
	 */
	arenas_extend(0);
	if (arenas[0] == NULL) {
		malloc_mutex_unlock(&init_lock);
		return (true);
	}

#ifndef NO_TLS
	/*
	 * Assign the initial arena to the initial thread, in order to avoid
	 * spurious creation of an extra arena if the application switches to
	 * threaded mode.
	 */
	arenas_map = arenas[0];
#endif

#ifdef JEMALLOC_MAG
	if (pthread_key_create(&mag_rack_tsd, thread_cleanup) != 0) {
		jemalloc_message("<jemalloc>",
		    ": Error in pthread_key_create()\n", "", "");
		abort();
	}
#endif
	/*
	 * Seed here for the initial thread, since choose_arena_hard() is only
	 * called for other threads.  The seed value doesn't really matter.
	 */
#ifdef JEMALLOC_BALANCE
	SPRN(balance, 42);
#endif

	malloc_spin_init(&arenas_lock);

	/* Get number of CPUs. */
	malloc_initializer = pthread_self();
	malloc_mutex_unlock(&init_lock);
	ncpus = malloc_ncpus();
	malloc_mutex_lock(&init_lock);

	if (ncpus > 1) {
		/*
		 * For SMP systems, create twice as many arenas as there are
		 * CPUs by default.
		 */
		opt_narenas_lshift++;
	}

	/* Determine how many arenas to use. */
	narenas = ncpus;
	if (opt_narenas_lshift > 0) {
		if ((narenas << opt_narenas_lshift) > narenas)
			narenas <<= opt_narenas_lshift;
		/*
		 * Make sure not to exceed the limits of what base_alloc() can
		 * handle.
		 */
		if (narenas * sizeof(arena_t *) > chunksize)
			narenas = chunksize / sizeof(arena_t *);
	} else if (opt_narenas_lshift < 0) {
		if ((narenas >> -opt_narenas_lshift) < narenas)
			narenas >>= -opt_narenas_lshift;
		/* Make sure there is at least one arena. */
		if (narenas == 0)
			narenas = 1;
	}
#ifdef JEMALLOC_BALANCE
	assert(narenas != 0);
	for (narenas_2pow = 0;
	     (narenas >> (narenas_2pow + 1)) != 0;
	     narenas_2pow++);
#endif

#ifdef NO_TLS
	if (narenas > 1) {
		static const unsigned primes[] = {1, 3, 5, 7, 11, 13, 17, 19,
		    23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83,
		    89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
		    151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
		    223, 227, 229, 233, 239, 241, 251, 257, 263};
		unsigned nprimes, parenas;

		/*
		 * Pick a prime number of hash arenas that is more than narenas
		 * so that direct hashing of pthread_self() pointers tends to
		 * spread allocations evenly among the arenas.
		 */
		assert((narenas & 1) == 0); /* narenas must be even. */
		nprimes = (sizeof(primes) >> SIZEOF_INT_2POW);
		parenas = primes[nprimes - 1]; /* In case not enough primes. */
		for (i = 1; i < nprimes; i++) {
			if (primes[i] > narenas) {
				parenas = primes[i];
				break;
			}
		}
		narenas = parenas;
	}
#endif

#ifndef NO_TLS
#  ifndef JEMALLOC_BALANCE
	next_arena = 0;
#  endif
#endif

	/* Allocate and initialize arenas. */
	arenas = (arena_t **)base_alloc(sizeof(arena_t *) * narenas);
	if (arenas == NULL) {
		malloc_mutex_unlock(&init_lock);
		return (true);
	}
	/*
	 * Zero the array.  In practice, this should always be pre-zeroed,
	 * since it was just mmap()ed, but let's be sure.
	 */
	memset(arenas, 0, sizeof(arena_t *) * narenas);
	/* Copy the pointer to the one arena that was already initialized. */
	arenas[0] = init_arenas[0];

	malloc_initialized = true;
	malloc_mutex_unlock(&init_lock);
	return (false);
}

/*
 * End general internal functions.
 */

#ifdef USE_JEMALLOC

/******************************************************************************/
/*
 * Begin malloc(3)-compatible functions.
 */

void *
malloc(size_t size)
{
	void *ret;

	if (malloc_init()) {
		ret = NULL;
		goto RETURN;
	}

	if (size == 0) {
#ifdef JEMALLOC_SYSV
		if (opt_sysv == false)
#endif
			size = 1;
#ifdef JEMALLOC_SYSV
		else {
			ret = NULL;
			goto RETURN;
		}
#endif
	}

	ret = imalloc(size);

RETURN:
	if (ret == NULL) {
#ifdef JEMALLOC_XMALLOC
		if (opt_xmalloc) {
			jemalloc_message("<jemalloc>",
			    ": Error in malloc(): out of memory\n", "",
			    "");
			abort();
		}
#endif
		errno = ENOMEM;
	}

	UTRACE(0, size, ret);
	return (ret);
}

int
posix_memalign(void **memptr, size_t alignment, size_t size)
{
	int ret;
	void *result;

	if (malloc_init())
		result = NULL;
	else {
		/* Make sure that alignment is a large enough power of 2. */
		if (((alignment - 1) & alignment) != 0
		    || alignment < sizeof(void *)) {
#ifdef JEMALLOC_XMALLOC
			if (opt_xmalloc) {
				jemalloc_message("<jemalloc>",
				    ": Error in posix_memalign(): "
				    "invalid alignment\n", "", "");
				abort();
			}
#endif
			result = NULL;
			ret = EINVAL;
			goto RETURN;
		}

		result = ipalloc(alignment, size);
	}

	if (result == NULL) {
#ifdef JEMALLOC_XMALLOC
		if (opt_xmalloc) {
			jemalloc_message("<jemalloc>",
			": Error in posix_memalign(): out of memory\n",
			"", "");
			abort();
		}
#endif
		ret = ENOMEM;
		goto RETURN;
	}

	*memptr = result;
	ret = 0;

RETURN:
	UTRACE(0, size, result);
	return (ret);
}

void *
calloc(size_t num, size_t size)
{
	void *ret;
	size_t num_size;

	if (malloc_init()) {
		num_size = 0;
		ret = NULL;
		goto RETURN;
	}

	num_size = num * size;
	if (num_size == 0) {
#ifdef JEMALLOC_SYSV
		if ((opt_sysv == false) && ((num == 0) || (size == 0)))
#endif
			num_size = 1;
#ifdef JEMALLOC_SYSV
		else {
			ret = NULL;
			goto RETURN;
		}
#endif
	/*
	 * Try to avoid division here.  We know that it isn't possible to
	 * overflow during multiplication if neither operand uses any of the
	 * most significant half of the bits in a size_t.
	 */
	} else if (((num | size) & (SIZE_T_MAX << (sizeof(size_t) << 2)))
	    && (num_size / size != num)) {
		/* size_t overflow. */
		ret = NULL;
		goto RETURN;
	}

	ret = icalloc(num_size);

RETURN:
	if (ret == NULL) {
#ifdef JEMALLOC_XMALLOC
		if (opt_xmalloc) {
			jemalloc_message("<jemalloc>",
			    ": Error in calloc(): out of memory\n", "",
			    "");
			abort();
		}
#endif
		errno = ENOMEM;
	}

	UTRACE(0, num_size, ret);
	return (ret);
}

void *
realloc(void *ptr, size_t size)
{
	void *ret;

	if (size == 0) {
#ifdef JEMALLOC_SYSV
		if (opt_sysv == false)
#endif
			size = 1;
#ifdef JEMALLOC_SYSV
		else {
			if (ptr != NULL)
				idalloc(ptr);
			ret = NULL;
			goto RETURN;
		}
#endif
	}

	if (ptr != NULL) {
		assert(malloc_initialized);

		ret = iralloc(ptr, size);

		if (ret == NULL) {
#ifdef JEMALLOC_XMALLOC
			if (opt_xmalloc) {
				jemalloc_message("<jemalloc>",
				    ": Error in realloc(): out of "
				    "memory\n", "", "");
				abort();
			}
#endif
			errno = ENOMEM;
		}
	} else {
		if (malloc_init())
			ret = NULL;
		else
			ret = imalloc(size);

		if (ret == NULL) {
#ifdef JEMALLOC_XMALLOC
			if (opt_xmalloc) {
				jemalloc_message("<jemalloc>",
				    ": Error in realloc(): out of "
				    "memory\n", "", "");
				abort();
			}
#endif
			errno = ENOMEM;
		}
	}

RETURN:
	UTRACE(ptr, size, ret);
	return (ret);
}

void
free(void *ptr)
{

	UTRACE(ptr, 0, 0);
	if (ptr != NULL) {
		assert(malloc_initialized);

		idalloc(ptr);
	}
}

/*
 * End malloc(3)-compatible functions.
 */
#endif /* USE_JEMALLOC */

/******************************************************************************/
/*
 * Begin non-standard functions.
 */

size_t
malloc_usable_size(const void *ptr)
{

	assert(ptr != NULL);

	return (isalloc(ptr));
}

/*
 * End non-standard functions.
 */
/******************************************************************************/
/*
 * Begin library-private functions.
 */

/******************************************************************************/
/*
 * Begin thread cache.
 */

/*
 * We provide an unpublished interface in order to receive notifications from
 * the pthreads library whenever a thread exits.  This allows us to clean up
 * thread caches.
 */
static void
thread_cleanup(void *arg)
{

#ifdef JEMALLOC_MAG
	assert((mag_rack_t *)arg == mag_rack);
	if (mag_rack != NULL) {
		assert(mag_rack != (void *)-1);
		mag_rack_destroy(mag_rack);
#ifdef JEMALLOC_DEBUG
		mag_rack = (void *)-1;
#endif
	}
#endif
}

/*
 * The following functions are used by threading libraries for protection of
 * malloc during fork().  These functions are only called if the program is
 * running in threaded mode, so there is no need to check whether the program
 * is threaded here.
 */

static void
jemalloc_prefork(void)
{
	bool again;
	unsigned i, j;
	arena_t *larenas[narenas], *tarenas[narenas];

	/* Acquire all mutexes in a safe order. */

	/*
	 * arenas_lock must be acquired after all of the arena mutexes, in
	 * order to avoid potential deadlock with arena_lock_balance[_hard]().
	 * Since arenas_lock protects the arenas array, the following code has
	 * to race with arenas_extend() callers until it succeeds in locking
	 * all arenas before locking arenas_lock.
	 */
	memset(larenas, 0, sizeof(arena_t *) * narenas);
	do {
		again = false;

		malloc_spin_lock(&arenas_lock);
		for (i = 0; i < narenas; i++) {
			if (arenas[i] != larenas[i]) {
				memcpy(tarenas, arenas, sizeof(arena_t *) *
				    narenas);
				malloc_spin_unlock(&arenas_lock);
				for (j = 0; j < narenas; j++) {
					if (larenas[j] != tarenas[j]) {
						larenas[j] = tarenas[j];
						malloc_spin_lock(
						    &larenas[j]->lock);
					}
				}
				again = true;
				break;
			}
		}
	} while (again);

	malloc_mutex_lock(&base_mtx);

	malloc_mutex_lock(&huge_mtx);

#ifdef JEMALLOC_DSS
	malloc_mutex_lock(&dss_mtx);
#endif
}

static void
jemalloc_postfork(void)
{
	unsigned i;
	arena_t *larenas[narenas];

	/* Release all mutexes, now that fork() has completed. */

#ifdef JEMALLOC_DSS
	malloc_mutex_unlock(&dss_mtx);
#endif

	malloc_mutex_unlock(&huge_mtx);

	malloc_mutex_unlock(&base_mtx);

	memcpy(larenas, arenas, sizeof(arena_t *) * narenas);
	malloc_spin_unlock(&arenas_lock);
	for (i = 0; i < narenas; i++) {
		if (larenas[i] != NULL)
			malloc_spin_unlock(&larenas[i]->lock);
	}
}

/*
 * End library-private functions.
 */
/******************************************************************************/
