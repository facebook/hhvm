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
