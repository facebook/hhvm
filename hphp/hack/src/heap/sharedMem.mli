(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This is just a sentinel for self-documenting purposes which some
    parts of the codebase use. They take a parameter "uses_sharedmem : SharedMem.uses"
    as a way to indicate to their callers that they read/write sharedmem. *)
type uses = Uses

exception Out_of_shared_memory

exception Hash_table_full

exception Heap_full

exception Sql_assertion_failure of int

exception C_assertion_failure of string

(** Configuration object that initializes shared memory. *)
type config = {
  global_size: int;
  heap_size: int;
  hash_table_pow: int;
  shm_use_sharded_hashtbl: bool;
  shm_cache_size: int;
  shm_dirs: string list;
  shm_min_avail: int;
  log_level: int;
  sample_rate: float;
  (* 0 - lz4, others -- compression level for zstd*)
  compression: int;
}
[@@deriving show]

(** Default configuration object *)
val default_config : config

(** Empty configuration object.

    There are places where we don't expect to write to shared memory, and doing
    so would be a memory leak. But since shared memory is global, it's very easy
    to accidentally call a function that will attempt such write. This config
    initializes shared memory with zero sizes. As such, attempting to write to
    shared memory that was initialized with this config, will make the program
    fail immediately. *)
val empty_config : config

(** A handle to initialized shared memory. Used to connect other workers to
    shared memory. *)
type handle

(** Internal type for a handle, to enable additional low-level heaps attachments **)
type internal_handle

(** Exposed for testing **)
val get_heap_size : handle -> int

(** Exposed for testing **)
val get_global_size : handle -> int

val clear_close_on_exec : handle -> unit

val set_close_on_exec : handle -> unit

val register_callbacks :
  (config -> num_workers:int -> internal_handle option) ->
  (internal_handle option -> worker_id:int -> unit) ->
  (unit -> internal_handle option) ->
  unit

(** Initialize shared memory.

    Must be called before forking. *)
val init : config -> num_workers:int -> handle

(** Connect other workers to shared memory *)
val connect : handle -> worker_id:int -> unit

(** Get the handle to shared memory. Returns nonsense if the current
    process hasn't yet connected to shared memory *)
val get_handle : unit -> handle

(** Allow or disallow remove operations. *)
val set_allow_removes : bool -> unit

(** Allow or disallow shared memory writes for the current process. *)
val set_allow_hashtable_writes_by_current_process : bool -> unit

(** Directly access the shared memory table.

    This can be used to provide proxying across the network *)
module RawAccess : sig
  type serialized = private bytes

  val mem_raw : string -> bool

  val get_raw : string -> serialized option

  val add_raw : string -> serialized -> unit

  val deserialize_raw : serialized -> 'a

  val serialize_raw : 'a -> serialized
end

(** Some telemetry utilities *)
module SMTelemetry : sig
  (** Get some shared-memory telemetry. Even works when shared memory hasn't
    been initialized yet. *)
  val get_telemetry : unit -> Telemetry.t

  (** Return the number of bytes allocated in shared memory. This includes
      bytes that were free'd but are not yet available for reuse. *)
  val heap_size : unit -> int

  (** Returns the number of bytes not reachable fro hashtable entries. *)
  val wasted_heap_size : unit -> int

  (** The logging level for shared memory statistics:
        - 0 = nothing
        - 1 = log totals, averages, min, max bytes marshalled and unmarshalled
    *)
  val hh_log_level : unit -> int

  (** Get the sample rate for shared memory statistics. *)
  val hh_sample_rate : unit -> float

  (** Get the number of used slots in our hashtable. *)
  val hash_used_slots : unit -> int * int

  (** Get the number of total slots in our hashtable. *)
  val hash_slots : unit -> int

  type table_stats = {
    nonempty_slots: int;
    used_slots: int;
    slots: int;
  }

  (** Combine [hash_used_slots] and [hash_slots] *)
  val hash_stats : unit -> table_stats

  (** Not sure. Return the removed number of entries? *)
  val hh_removed_count : unit -> int

  (** Did we overflow the heap? *)
  val is_heap_overflow : unit -> bool

  (** Compute the size of values in the garbage-collected heap. (???) *)
  val value_size : Obj.t -> int

  (** Log to our telemetry infra that we successfully initialized shared
      memory *)
  val init_done : unit -> unit
end

(** Interface to the garbage collector *)
module GC : sig
  val should_collect : [ `aggressive | `always_TEST | `gentle ] -> bool

  val collect : [ `aggressive | `always_TEST | `gentle ] -> unit
end

(** A hasher that hashes user-defined keys. The resulting hash can be used
    to index the big shared-memory table.

    Each hash is built by concatenating an optional "old" prefix, a heap-prefix
    and an object-specific key, then hashing the concatenation.

    The unique heap-prefix is automatically generated when calling `MakeKeyHasher`.

    Currently we use MD5 as the hashing algorithm. Note that only the first
    8 bytes are used to index the shared memory table. *)
module type KeyHasher = sig
  (** The type of keys that OCaml-land callers try to insert.

      This key will be object-specific (unique within a heap), but might not be
      unique across heaps. *)
  type key

  (** The hash of an old or new key.

      This hash will be unique across all heaps. *)
  type hash

  val hash : key -> hash

  val hash_old : key -> hash

  (** Return the raw bytes of the digest. Note that this is not hex encoded. *)
  val to_bytes : hash -> string
end

(** The interface that all keys need to implement *)
module type Key = sig
  type t

  val to_string : t -> string

  val compare : t -> t -> int
end

(** Make a new key that can be stored in shared-memory. *)
module MakeKeyHasher (Key : Key) : KeyHasher with type key = Key.t

(** The interface that all values need to implement *)
module type Value = sig
  type t

  val description : string
end

(** Whether or not a backend is evictable. *)
module type Evictability = sig
  val evictable : bool
end

(** Used to indicate that values can be evicted at all times. *)
module Evictable : Evictability

(** used to indicate that values must never be evicted from the backend. *)
module NonEvictable : Evictability

(** Module type for a shared-memory backend for a heap.

    Each backend provided raw access to the underlying shared hash table. *)
module type Backend = functor (KeyHasher : KeyHasher) (Value : Value) -> sig
  val add : KeyHasher.hash -> Value.t -> unit

  val mem : KeyHasher.hash -> bool

  val get : KeyHasher.hash -> Value.t option

  val remove : KeyHasher.hash -> unit

  val move : KeyHasher.hash -> KeyHasher.hash -> unit
end

(** Backend that provides immediate access to the underlying hashtable. *)
module ImmediateBackend (_ : Evictability) : Backend

(** A heap for a user-defined type.

    Each heap supports "old" and "new" values.

    There are several cases where we need to compare the old and the new
    representations of objects to determine what has changed.

    The "old" representation is the value that was bound to that key in
    the last round of type-checking. *)
module type Heap = sig
  type key

  type value

  (** [KeyHasher] created for this heap.

      A new [KeyHasher] with a unique prefix is automatically generated
      for each heap. Normally, you shouldn't have to use the [KeyHasher]
      directly, but Zoncolan does. *)
  module KeyHasher : KeyHasher with type key = key

  module KeySet : Set.S with type elt = key

  module KeyMap : WrappedMap.S with type key = key

  (** Adds a binding to the table.

      Note: TODO(hverr), currently the semantics of inserting a value for a key
      that's already in the heap are unclear and depend on whether you have a
      local-changes stack or not. *)
  val add : key -> value -> unit

  val get : key -> value option

  val get_old : key -> value option

  val get_batch : KeySet.t -> value option KeyMap.t

  val get_old_batch : KeySet.t -> value option KeyMap.t

  val remove : key -> unit

  val remove_old : key -> unit

  val remove_batch : KeySet.t -> unit

  val remove_old_batch : KeySet.t -> unit

  val mem : key -> bool

  val mem_old : key -> bool

  (** Equivalent to moving a set of entries (= key + value) to some heap of old entries. *)
  val oldify_batch : KeySet.t -> unit

  val revive_batch : KeySet.t -> unit

  module LocalChanges : sig
    val push_stack : unit -> unit

    val pop_stack : unit -> unit
  end
end

(** A heap for a user-defined type.

    Provides no worker-local caching. Directly stores to and queries from
    shared memory. *)
module Heap (_ : Backend) (Key : Key) (Value : Value) :
  Heap
    with type key = Key.t
     and type value = Value.t
     and module KeyHasher = MakeKeyHasher(Key)
     and module KeySet = Set.Make(Key)
     and module KeyMap = WrappedMap.Make(Key)

(** A worker-local cache layer.

    Each local cache defines its own eviction strategy.
    For example, we currently have [FreqCache] and [OrderedCache]. *)
module type LocalCacheLayer = sig
  type key

  type value

  val add : key -> value -> unit

  val get : key -> value option

  val remove : key -> unit

  val clear : unit -> unit

  val get_telemetry_items_and_keys : unit -> Obj.t * key Seq.t
end

(** Invalidate all worker-local caches *)
val invalidate_local_caches : unit -> unit

(** Capacity of a worker-local cache.

    In number of elements. *)
module type Capacity = sig
  val capacity : int
end

(** FreqCache is an LFU (Least Frequently Used) cache.

    It keeps count of how many times each item in the cache has been
    added/replaced/fetched and, when it reaches 2*capacity, then it
    flushes 1*capacity items and they lose their counts. This might result
    in a lucky few early items getting to stay in the cache while newcomers
    get evicted...

    It is Hashtbl.t-based with a bounded number of elements. *)
module FreqCache (Key : Key) (Value : Value) (_ : Capacity) :
  LocalCacheLayer with type key = Key.t and type value = Value.t

(** OrderedCache is an LRA (Least Recently Added) cache.

    Whenever you add an item beyond capacity, it will evict the oldest one
    to be added.

    It is Hashtbl.t-based with a bounded number of elements. *)
module OrderedCache (Key : Key) (Value : Value) (_ : Capacity) :
  LocalCacheLayer with type key = Key.t and type value = Value.t

(** Same as [Heap] but provides a layer of worker-local caching. *)
module HeapWithLocalCache
    (_ : Backend)
    (Key : Key)
    (Value : Value)
    (_ : Capacity) : sig
  include
    Heap
      with type key = Key.t
       and type value = Value.t
       and module KeyHasher = MakeKeyHasher(Key)
       and module KeySet = Set.Make(Key)
       and module KeyMap = WrappedMap.Make(Key)

  val write_around : key -> value -> unit

  val get_no_cache : key -> value option

  module Cache : LocalCacheLayer with type key = key and type value = value
end
