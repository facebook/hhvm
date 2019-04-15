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
static color_t ocamlpool_color;

static int ocamlpool_allocated_chunks_counter = 0;
static size_t ocamlpool_next_chunk_size = OCAMLPOOL_DEFAULT_SIZE;

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
    ocamlpool_color = caml_allocation_color((void*)ocamlpool_root);

  ocamlpool_in_section = 1;
  ocamlpool_sane_young_ptr = caml_young_ptr;

  assert_in_section();
}


void ocamlpool_leave(void)
{
  assert_in_section();

  ocamlpool_in_section = 0;

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

/* Unused memory from current chunk */
size_t ocamlpool_chunk_remaining_words(void)
{
  if (ocamlpool_root == Val_unit)
    return 0;
  return Wosize_val(ocamlpool_root);
}

/* Return the current chunk to OCaml memory system.
 * FIXME: For now, the chunk is just strongly referenced during used and
 *        unreferenced when released.
 *        Improvements:
 *        - make it weak so that OCaml GC can grab it under memory pressure
 *        - add it to freelist on release, so that memory can be reclaimed
 *          before next GC.
 */
void ocamlpool_chunk_release(void)
{
  ocamlpool_root = Val_unit;
}

#define OCAMLPOOL_SET_HEADER(v, size, tag, color) \
  *((header_t*)(((value*)(v)) - 1)) = \
    Make_header_allocated_here(size, tag, color);

static value ocamlpool_chunk_truncate(size_t word_size)
{
  assert_in_section();
  ocamlpool_assert(ocamlpool_root != Val_unit);
  abort_unless((Wosize_val(ocamlpool_root) >= word_size) && (word_size >= 1));

  OCAMLPOOL_SET_HEADER(ocamlpool_root, word_size, String_tag, ocamlpool_color);
  value *first_word = (value*)ocamlpool_root;
  first_word[word_size - 1] = 0;
  return (value)(first_word + word_size + 1);
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
  /* Make it look like a well-formed string */
  OCAMLPOOL_SET_HEADER(ocamlpool_root, words - 1, String_tag, ocamlpool_color);
  caml_add_to_heap(block);

  caml_allocated_words += words;
}

/* OCaml value allocations
 * ===========================================================================
 *
 * A fast to reserve OCaml memory when inside ocamlpool section.
 */

value ocamlpool_reserve_block(int tag, size_t words)
{
  assert_in_section();

  if (words == 0) return Atom(tag);

  size_t size = words + 1;
  size_t remaining = ocamlpool_chunk_remaining_words();
  abort_unless(size < ocamlpool_next_chunk_size);
  /* We cannot allocate a value bigger than the chunk size */

  value result;

  /* Easy case: we have enough remaining space */
  if (size < remaining)
  {
    result = ocamlpool_chunk_truncate(remaining - size);
  }
  /* Exactly the space we want */
  else if (words == remaining)
  {
    result = ocamlpool_root;
    ocamlpool_root = Val_unit;
  }
  /* Two reasons to get there:
   * - words - 1 == remaining:
   *     splitting the block would introduce fragmentation that is annoying to
   *     deal with.
   * - words > remaining:
   *   not enough space, allocate a new chunk
   */
  else
  {
    ocamlpool_chunk_alloc();
    result =
      ocamlpool_chunk_truncate(ocamlpool_chunk_remaining_words() - size);
  }

  OCAMLPOOL_SET_HEADER(result, words, tag, ocamlpool_color);
  for (size_t i = 0; i < words; ++i)
  {
    Field(result, i) = Val_unit;
  }
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
