(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Determine whether a global constant with the given name is declared in
the reverse naming table. *)
val const_exists : string -> bool

(** Look up the file path at which the given global constant was declared in
the reverse naming table. *)
val get_const_path : string -> Relative_path.t option

(** Look up the position at which the given global constant was declared in
the reverse naming table. *)
val get_const_pos : string -> FileInfo.pos option

(** Record that a global constant with the given name was declared at the
given position. *)
val add_const : string -> FileInfo.pos -> unit

(** Remove all global constants with the given names from the reverse naming
table. *)
val remove_const_batch : SSet.t -> unit

(** Determine whether a global function with the given name is declared in
the reverse naming table. *)
val fun_exists : string -> bool

(** Look up the file path in which the given global function was declared in
the reverse naming table. *)
val get_fun_path : string -> Relative_path.t option

(** Look up the position at which the given global function was declared in
the reverse naming table. *)
val get_fun_pos : string -> FileInfo.pos option

(** Look up the canonical name for the given global function. *)
val get_fun_canon_name : Provider_context.t -> string -> string option

(** Record that a global function with the given name was declared at the
given position. *)
val add_fun : string -> FileInfo.pos -> unit

(** Remove all global functions with the given names from the reverse naming
table. *)
val remove_fun_batch : SSet.t -> unit

(** Record that a type (one of [Naming_types.kind_of_type] was declared at
the given position. These types all live in the same namespace, unlike
functions and constants. *)
val add_type : string -> FileInfo.pos -> Naming_types.kind_of_type -> unit

(** Look up the position at which the given type was declared in the reverse
naming table. *)
val get_type_pos : string -> FileInfo.pos option
