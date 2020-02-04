(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** state is a global mutable variable, accumulating all counts *)
type state

(** reset will zero all counters, adjust the global mutable state,
and return the previous state. You should 'restore_state' when done,
in case your caller had been doing their own count.
If 'enable' is false then no counting will happen. *)
val reset : enable:bool -> state

(** restores global mutable state to what it was before you called 'reset' *)
val restore_state : state -> unit

val count_decl_accessor : (unit -> 'a) -> 'a

val count_disk_cat : (unit -> 'a) -> 'a

val count_get_ast : (unit -> 'a) -> 'a

val get_counters : unit -> Telemetry.t
