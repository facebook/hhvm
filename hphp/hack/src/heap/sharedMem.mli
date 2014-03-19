(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* The heap shared across all the processes.
 *
 * The Heap is not exposed directly to the user (cf shared.mli),
 * because we don't want to mix values of different types. Instead, we want
 * to use a functor.
 *)
(*****************************************************************************)
open Utils

(*****************************************************************************)
(* Initializes the shared memory. Must be called before forking! *)
(*****************************************************************************)

val init: unit -> unit

(*****************************************************************************)
(* The shared memory garbage collector. It must be called every time we
 * free data (cf hh_shared.c for the underlying C implementation).
 *)
(*****************************************************************************)

val collect: unit -> unit

(*****************************************************************************)
(* Must be called after the initialization of the hack server is over.
 * (cf serverInit.ml).
 *)
(*****************************************************************************)

val init_done: unit -> unit

(*****************************************************************************)
(* Cache invalidation. *)
(*****************************************************************************)

val invalidate_caches: unit -> unit

(*****************************************************************************)
(* The signature of a shared memory hashtable.
 * To create one: SharedMem.NoCache(struct type = my_type_of_value end).
 * The call to Make will create a hashtable in shared memory (visible to
 * all the workers).
 * Use NoCache/WithCache if you want caching or not.
 * If you do, bear in mind that the cache must be maintained by the caller.
 * So you will have to invalidate the caches yourself.
 *)
(*****************************************************************************)

module type S = sig
  type t

  (* Safe for concurrent writes, the first writer wins, the second write
   * is dismissed.
   *)
  val add: string -> t -> unit

  (* Safe for concurrent reads, but not if interleaved with any operation
   * mutating the table (add, remove etc ..).
   *)
  val get: string -> t option
  val get_old: string -> t option
  val get_old_batch: SSet.t -> t option SMap.t
  val remove_old_batch: SSet.t -> unit
  val find_unsafe: string -> t
  val get_batch: SSet.t -> t option SMap.t
  val remove_batch: SSet.t -> unit

  (* Safe for concurrent access. *)
  val mem: string -> bool

  (* This function takes the elements present in the set and keep the "old"
   * version in a separate heap. This is useful when we want to compare 
   * what has changed. We will be in a situation for type-checking 
   * (cf typing/typing_redecl_service.ml) where we want to compare the type
   * of a class in the previous environment vs the current type.
   *)
  val oldify_batch: SSet.t -> unit
  (* Reverse operation of oldify *)
  val revive_batch: SSet.t -> unit
end

module NoCache  : functor (Value:Value.Type) -> S with type t = Value.t
module WithCache: functor (Value:Value.Type) -> S with type t = Value.t
