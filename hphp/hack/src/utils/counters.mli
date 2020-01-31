(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  (* how many times did 'count' get called? *)
  count: int;
  (* cumulative duration of all calls to 'count' *)
  time: float;
  (* an internal field to avoid double-counting when a method calls 'count' and so does a nested one *)
  is_counting: bool;
}

(** state is a global mutable variable, accumulating all counts *)
type state

(** reset will zero all counters, adjust the global mutable state,
and return the previous state. You should 'restore_state' when done,
in case your caller had been doing their own count.
If 'enable' is false then no counting will happen. *)
val reset : enable:bool -> state

(** restores global mutable state to what it was before you called 'reset' *)
val restore_state : state -> unit
