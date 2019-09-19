#ifndef OCAMLPOOL_H
#define OCAMLPOOL_H

#include <caml/mlvalues.h>

/*
 * FIXME: The current system always maintain the heap in a well formed state,
 *        making the current pool look like a string to the OCaml GC and
 *        fragmenting it during allocation.
 *        This is not necessary, it should be correct to just keep a pointer
 *        and the size of the unallocated area while in the section and make
 *        it look like a string when leaving the section.
 * FIXME: The current chunking system might be incorrect if the incremental
 *        scan stops in the middle of the unallocated chunk.
 *        To prevent that, this chunk is marked as non-scannable (a string),
 *        but I should double check the behavior of Obj.truncate.
 * FIXME: For now, the chunk is just strongly referenced during used and
 *        unreferenced when released.
 *        Improvements:
 *        - make it weak so that OCaml GC can grab it under memory pressure
 *        - add it to freelist on release, so that memory can be reclaimed
 *          before next GC.
 */

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

void ocamlpool_enter(void);
void ocamlpool_leave(void);

/* Memory chunking
 * ===========================================================================
 *
 * Pool memory is allocated by large chunks.
 * The default settings should work well, though it is possible to tweak
 * and monitor these parameters.
 *
 */

/* Number of chunks allocated by ocamlpool since beginning of execution */
int ocamlpool_allocated_chunks(void);

/* Controlling the size of allocated chunks.
 * >= 512, preferably >= 2^20 */
#define OCAMLPOOL_DEFAULT_SIZE (1024 * 1024)
size_t ocamlpool_get_next_chunk_size(void);
void ocamlpool_set_next_chunk_size(size_t sz);

/* Return the current chunk to OCaml memory system */
void ocamlpool_chunk_release(void);

/* OCaml value allocations
 * ===========================================================================
 *
 * A fast to reserve OCaml memory when inside ocamlpool section.
 */

value ocamlpool_reserve_string(size_t bytes);
value ocamlpool_reserve_block(int tag, size_t words);

extern color_t ocamlpool_color;
extern value *ocamlpool_limit, *ocamlpool_cursor, *ocamlpool_bound;;
extern uintnat ocamlpool_generation;

#endif /*!OCAMLPOOL_H*/
