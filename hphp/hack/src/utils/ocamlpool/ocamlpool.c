#define CAML_NAME_SPACE

#include "ocamlpool.h"
#include <caml/version.h>

#if OCAML_VERSION < 40700

extern value *caml_young_ptr;
extern uintnat caml_allocated_words;
typedef struct {
  void *block;           /* address of the malloced block this chunk lives in */
  asize_t alloc;         /* in bytes, used for compaction */
  asize_t size;          /* in bytes */
  char *next;
} heap_chunk_head;

#define Chunk_size(c) (((heap_chunk_head *) (c)) [-1]).size

#else

#define CAML_INTERNALS

#endif

#include <caml/memory.h>
#include <caml/gc.h>
#include <caml/major_gc.h>

/* Global state
 * ===========================================================================
 */

/* An OCaml root pointing to the current chunk, or Val_unit */
static value ocamlpool_root = Val_unit;

/* 1 iff we are inside an ocamlpool section */
static int ocamlpool_in_section = 0;

/* For sanity checks, caml_young_ptr is copied when entering the section.
 * While inside the section, we check that the value has not changed as this
 * would result in difficult to track bugs. */
static void *ocamlpool_sane_young_ptr;

/* The GC color to give to blocks that are allocated inside the pool.
 * Updated when allocating a new chunk and when entering the section.  */
color_t ocamlpool_color = 0;

static int ocamlpool_allocated_chunks_counter = 0;
static size_t ocamlpool_next_chunk_size = OCAMLPOOL_DEFAULT_SIZE;

/* */
uintnat ocamlpool_generation = 0;
value *ocamlpool_limit = 0, *ocamlpool_cursor = 0, *ocamlpool_bound = 0;

/* Sanity checks
 * ===========================================================================
 *
 * Contracts checking that the invariants are maintained inside the library
 * and that the API is used correctly.
 */

static void abort_unless(int x)
{
  if (!x) abort();
}

#define WORD_SIZE (sizeof(void*))
#define IS_WORD_ALIGNED(x) (((x) & (WORD_SIZE - 1)) == 0)

#ifndef OCAMLPOOL_NO_ASSERT

static void ocamlpool_assert(int x)
{
  abort_unless(x);
}

#else

static void ocamlpool_assert(int x)
{
  (void)x;
}

#endif

static void assert_in_section(void)
{
  ocamlpool_assert(ocamlpool_in_section == 1 &&
                   ocamlpool_sane_young_ptr == caml_young_ptr);
}

static void assert_out_of_section(void)
{
  ocamlpool_assert(ocamlpool_in_section == 0);
}

#define OCAMLPOOL_SET_HEADER(v, size, tag, color) \
  *((header_t*)(((value*)(v)) - 1)) = \
    Make_header_allocated_here(size, tag, color);

/* OCamlpool sections
 * ===========================================================================
 *
 * Inside the section, the OCaml heap will be in an invalid state.
 * OCaml runtime functions should not be called.
 *
 * Since the GC will never run while in an OCaml pool section,
 * it is safe to keep references to OCaml values as long as these does not
 * outlive the section.
 */

static void init_cursor(void)
{
  ocamlpool_limit = (value*)ocamlpool_root + 1;
  ocamlpool_bound = (value*)ocamlpool_root + Wosize_val(ocamlpool_root);
  ocamlpool_cursor = ocamlpool_bound;
}

static void ocamlpool_chunk_alloc(void);

static void ocamlpool_chunk_truncate(void)
{
  if (ocamlpool_root == Val_unit)
    return;

  size_t word_size = (value*)ocamlpool_cursor - (value*)ocamlpool_root;

  OCAMLPOOL_SET_HEADER(ocamlpool_root, word_size, String_tag, ocamlpool_color);
  value *first_word = (value*)ocamlpool_root;
  first_word[word_size - 1] = 0;
}

void ocamlpool_enter(void)
{
  assert_out_of_section();

  static int ocamlpool_initialized = 0;
  if (ocamlpool_initialized == 0)
  {
    ocamlpool_initialized = 1;
    caml_register_global_root(&ocamlpool_root);
  }

  if (ocamlpool_root != Val_unit)
  {
    ocamlpool_color = caml_allocation_color((void*)ocamlpool_root);
    init_cursor();
  }
  else
  {
    ocamlpool_chunk_alloc();
  }

  ocamlpool_in_section = 1;
  ocamlpool_sane_young_ptr = caml_young_ptr;

  assert_in_section();
}


void ocamlpool_leave(void)
{
  if (ocamlpool_in_section != 1) {
    return;
  }

  assert_in_section();

  ocamlpool_chunk_truncate();
  ocamlpool_in_section = 0;
  ocamlpool_limit = 0;
  ocamlpool_bound = 0;
  ocamlpool_cursor = 0;

  ocamlpool_generation += 1;

  assert_out_of_section();
}

/* Memory chunking
 * ===========================================================================
 *
 * Pool memory is allocated by large chunks.
 * The default settings should work well, though it is possible to tweak
 * and monitor these parameters.
 *
 * FIXME: The current chunking system might be incorrect if the incremental
 *        scan stops in the middle of the unallocated chunk.
 *        To prevent that, this chunk is marked as non-scannable (a string),
 *        but I should double check the behavior of Obj.truncate.
 */

/* Number of chunks allocated by ocamlpool since beginning of execution */
int ocamlpool_allocated_chunks(void)
{
  return ocamlpool_allocated_chunks_counter;
}

/* Controlling the size of allocated chunks.
 * Must be multiple of sizeof(void*), preferably >= 2^20. */
size_t ocamlpool_get_next_chunk_size(void)
{
  return ocamlpool_next_chunk_size;
}

void ocamlpool_set_next_chunk_size(size_t sz)
{
  abort_unless(IS_WORD_ALIGNED(sz));
  ocamlpool_next_chunk_size = sz;
}

void ocamlpool_chunk_release(void)
{
  ocamlpool_chunk_truncate();
  ocamlpool_limit = 0;
  ocamlpool_bound = 0;
  ocamlpool_cursor = 0;
  ocamlpool_root = Val_unit;
}

static void ocamlpool_chunk_alloc(void)
{
  ocamlpool_assert(ocamlpool_next_chunk_size > 1);
  void *block = caml_alloc_for_heap(ocamlpool_next_chunk_size * WORD_SIZE);
  abort_unless(block != NULL);

  size_t chunk_size = Chunk_size(block);
  ocamlpool_assert(IS_WORD_ALIGNED(chunk_size));

  ocamlpool_color = caml_allocation_color(block);
  ocamlpool_allocated_chunks_counter += 1;

  size_t words = (chunk_size / WORD_SIZE);

  ocamlpool_root = (value)((value*)block + 1);
  OCAMLPOOL_SET_HEADER(ocamlpool_root, words - 1, String_tag, ocamlpool_color);
  init_cursor();
  caml_add_to_heap(block);

  caml_allocated_words += words;
}

/* OCaml value allocations
 * ===========================================================================
 *
 * A fast way to reserve OCaml memory when inside ocamlpool section.
 */
value ocamlpool_reserve_block(int tag, size_t words)
{
  size_t size = words + 1;
  value *pointer = ocamlpool_cursor - size;

  if (pointer < ocamlpool_limit || pointer >= ocamlpool_bound)
  {
    size_t old_ocamlpool_next_chunk_size = ocamlpool_next_chunk_size;
    if (size >= ocamlpool_next_chunk_size) {
        ocamlpool_next_chunk_size = size;
    }
    ocamlpool_chunk_truncate();
    ocamlpool_chunk_alloc();
    ocamlpool_next_chunk_size = old_ocamlpool_next_chunk_size;
    pointer = ocamlpool_cursor - size;
    abort_unless(pointer >= ocamlpool_limit);
  }

  ocamlpool_cursor = pointer;
  value result = (value)(pointer + 1);

  OCAMLPOOL_SET_HEADER(result, words, tag, ocamlpool_color);
  return result;
}

value ocamlpool_reserve_string(size_t bytes)
{
  assert_in_section();

  size_t words =
    ((bytes + 1 /*null-ending*/ + (WORD_SIZE-1)/*rounding*/) / WORD_SIZE);
  size_t length = (words * WORD_SIZE);

  value result = ocamlpool_reserve_block(String_tag, words);

  ((value*)result)[words - 1] = 0;
  ((char*)result)[length - 1] = length - bytes - 1;

  return result;
}
