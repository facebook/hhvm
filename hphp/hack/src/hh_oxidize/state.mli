(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val curr_module_name : unit -> string
(** The name of the module currently being converted. *)

val with_module_name : string -> (unit -> 'a) -> 'a
(** Run the given function in a context where {!curr_module_name} will return
    the given module name. Not re-entrant. *)

val self : unit -> string
(** The name of the type currently being converted. *)

val with_self : string -> (unit -> 'a) -> 'a
(** Run the given function in a context where {!self} will return the given type
    name. Not re-entrant. *)
