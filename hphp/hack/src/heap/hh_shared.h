#ifndef HH_SHARED_H
#define HH_SHARED_H

#define CAML_NAME_SPACE
#include <caml/mlvalues.h>

/*****************************************************************************/
/* Initialization & connection. */
/*****************************************************************************/
/* Initializes the shared heap. */
/* Must be called by the master BEFORE forking the workers! */
CAMLprim value hh_shared_init(
    value config_val,
    value shm_dir_val,
    value num_workers_val
);
value hh_check_heap_overflow(void);
/* Must be called by every worker before any operation is performed. */
value hh_connect(value connector, value worker_id_val);
/* Can only be called after init or after earlier connect. */
value hh_get_handle(void);

/*****************************************************************************/
/* Heap diagnostics. */
/*****************************************************************************/
CAMLprim value hh_used_heap_size(void);
CAMLprim value hh_wasted_heap_size(void);
CAMLprim value hh_log_level(void);
CAMLprim value hh_sample_rate(void);
CAMLprim value hh_hash_used_slots(void);
CAMLprim value hh_hash_slots(void);

/* Provides a counter which increases over the lifetime of the program
 * including all forks. Uses a global until hh_shared_init is called.
 * Safe to use in the early init stages of the program, as long as you fork
 * after hh_shared_init. Wraps around at the maximum value of an ocaml int.
 */
CAMLprim value hh_counter_next(void);

/*****************************************************************************/
/* Worker management. */
/*****************************************************************************/
CAMLprim value hh_stop_workers(void);
CAMLprim value hh_resume_workers(void);
CAMLprim value hh_raise_if_should_exit(void);
CAMLprim value hh_should_exit (void);
CAMLprim value hh_set_can_worker_stop(value val);
CAMLprim value hh_malloc_trim(void);
CAMLprim value hh_set_allow_removes(value val);
CAMLprim value hh_set_allow_hashtable_writes_by_current_process(value val);

/*****************************************************************************/
/* Global storage. */
/*****************************************************************************/
void hh_shared_store(value data);
CAMLprim value hh_shared_load(void);
void hh_shared_clear(void);

/*****************************************************************************/
/* Garbage collection. */
/*****************************************************************************/
CAMLprim value hh_collect(void);

/*****************************************************************************/
/* Deserialization. */
/*****************************************************************************/
/* Returns the value associated to a given key, and deserialize it. */
/* The key MUST be present. */
CAMLprim value hh_get_and_deserialize(value key);

/*****************************************************************************/
/* Raw access for network proxying.
   hh_get_raw key |> hh_deserialize_raw = hh_get key
   hh_serialize_raw data |> hh_add_raw key = hh_add key data
 */
/*****************************************************************************/
/* The key MUST be present. */
CAMLprim value hh_get_raw(value key);
CAMLprim value hh_add_raw(value key, value heap_entry);
CAMLprim value hh_serialize_raw(value data);
CAMLprim value hh_deserialize_raw(value heap_entry);


/*****************************************************************************/
/* Hashtable operations. */
/*****************************************************************************/
/* Returns the size of the value associated to a given key.
 * The key MUST be present.
 */
CAMLprim value hh_get_size(value key);
/* Adds a key/value pair to the hashtable. Returns the number of bytes
 * allocated in the heap, or a negative number if no memory was allocated. */
value hh_add(value evictable, value key, value data);
/* Returns true if the key is present in the hashtable. */
value hh_mem(value key);
/* The following operations are only to be performed by the master. */
/* Moves the data associated to key1 to key2.
 * key1 must be present. key2 must be free.
 */
void hh_move(value key1, value key2);
/* Removes a key from the hash table. */
CAMLprim value hh_remove(value key);

/*****************************************************************************/
/* Utility */
/*****************************************************************************/
/* Get the hash of a string, based on MD5. */
CAMLprim value hh_get_hash_ocaml(value key);
/* This assert will fail if the current process is not the master process. */
CAMLprim value hh_assert_master(void);

#endif
