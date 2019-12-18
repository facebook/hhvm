(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type size = int

module type Entry = sig
  type key

  type value

  (** Get the size associated with a [value]. For example, you can measure its
  size in bytes. If the size is always [1], this causes the cache to act as a
  regular LRU cache.

  The [key] is currently not considered as part of the size. *)
  val get_size : value -> size
end

module Cache (Entry : Entry) : sig
  type t

  (** The cache keeps a mutable record of its performance. You can reset it. *)
  type telemetry = {
    time_spent: float;
    peak_size: size;
    num_evictions: int;
  }

  (** Construct a new cache which can store up to [max_size] of values. *)
  val make : max_size:size -> t

  (** Remove all entries from the cache. *)
  val clear : t -> unit

  (** Get the number of elements currently in the cache. *)
  val length : t -> int

  (** Add a [key]-[value] pair to the cache.

  The cache is always resized to fit under the memory limit after any addition
  operation. Under some circumstances, this could mean that the given [value] is
  immediately evicted. (For example, if the [value] is greater than the maximum
  size of the cache, then it must be evicted.) *)
  val add : t -> key:Entry.key -> value:Entry.value -> unit

  (** Find the element with the given [key] in the cache and return the
  corresponding value. If the [key] is not present, calls [default] to calculate
  its value, then [add]s it to the cache and returns that value.

  The value is always guaranteed to be returned (whether by lookup or
  calculation), although it may be evicted immediately from the cache (see note on
  [add]). *)
  val find_or_add :
    t -> key:Entry.key -> default:(unit -> Entry.value) -> Entry.value

  (** Remove the entry with the given key from the cache. If the key is not
  present, does nothing. *)
  val remove : t -> key:Entry.key -> unit

  (** The cache keeps track of how long it's spent doing cache overhead *)
  val get_telemetry : t -> telemetry

  (** You can reset the timer. *)
  val reset_telemetry : t -> unit
end
