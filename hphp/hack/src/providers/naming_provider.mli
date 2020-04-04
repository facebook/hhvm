(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Determine whether a global constant with the given name is declared in
the reverse naming table. *)
val const_exists : Provider_context.t -> string -> bool

(** Look up the file path at which the given global constant was declared in
the reverse naming table. *)
val get_const_path : Provider_context.t -> string -> Relative_path.t option

(** Look up the position at which the given global constant was declared in
the reverse naming table. *)
val get_const_pos : Provider_context.t -> string -> FileInfo.pos option

(** Record that a global constant with the given name was declared at the
given position. *)
val add_const : Provider_backend.t -> string -> FileInfo.pos -> unit

(** Remove all global constants with the given names from the reverse naming
table. *)
val remove_const_batch : Provider_backend.t -> SSet.t -> unit

(** Determine whether a global function with the given name is declared in
the reverse naming table. *)
val fun_exists : Provider_context.t -> string -> bool

(** Look up the file path in which the given global function was declared in
the reverse naming table. *)
val get_fun_path : Provider_context.t -> string -> Relative_path.t option

(** Look up the position at which the given global function was declared in
the reverse naming table. *)
val get_fun_pos : Provider_context.t -> string -> FileInfo.pos option

(** Look up the canonical name for the given global function. *)
val get_fun_canon_name : Provider_context.t -> string -> string option

(** Record that a global function with the given name was declared at the
given position. *)
val add_fun : Provider_backend.t -> string -> FileInfo.pos -> unit

(** Remove all global functions with the given names from the reverse naming
table. *)
val remove_fun_batch : Provider_backend.t -> SSet.t -> unit

(** Record that a type (one of [Naming_types.kind_of_type] was declared at
the given position. These types all live in the same namespace, unlike
functions and constants. *)
val add_type :
  Provider_backend.t ->
  string ->
  FileInfo.pos ->
  Naming_types.kind_of_type ->
  unit

(** Remove all types with the given names from the reverse naming table. *)
val remove_type_batch : Provider_backend.t -> SSet.t -> unit

(** Look up the position at which the given type was declared in the reverse
naming table. *)
val get_type_pos : Provider_context.t -> string -> FileInfo.pos option

(** Look up the file path declaring the given type in the reverse naming
table. *)
val get_type_path : Provider_context.t -> string -> Relative_path.t option

(** Look up the kind with which the given type was declared in the reverse
naming table. *)
val get_type_kind :
  Provider_context.t -> string -> Naming_types.kind_of_type option

(** Look up the position and kind with which the given type was declared in
the reverse naming table. *)
val get_type_pos_and_kind :
  Provider_context.t ->
  string ->
  (FileInfo.pos * Naming_types.kind_of_type) option

(** Look up the path and kind with which the given type was declared in the
reverse naming table. *)
val get_type_path_and_kind :
  Provider_context.t ->
  string ->
  (Relative_path.t * Naming_types.kind_of_type) option

(** Look up the canonical name for the given type. *)
val get_type_canon_name : Provider_context.t -> string -> string option

(** Look up the file path declaring the given class in the reverse naming
table. Same as calling [get_type_pos] and extracting the path if the result
is a [Naming_types.TClass]. *)
val get_class_path : Provider_context.t -> string -> Relative_path.t option

(** Record that a class with the given name was declared at the given
position. Same as calling [add_type] with [Naming_types.TClass].
*)
val add_class : Provider_backend.t -> string -> FileInfo.pos -> unit

(** Look up the file path declaring the given class in the reverse naming
table. Same as calling [get_type_pos] and extracting the path if the result
is a [Naming_types.TRecordDef]. *)
val get_record_def_path : Provider_context.t -> string -> Relative_path.t option

(** Record that a class with the given name was declared at the given
position. Same as calling [add_type] with [Naming_types.TRecordDef].
*)
val add_record_def : Provider_backend.t -> string -> FileInfo.pos -> unit

(** Look up the file path declaring the given class in the reverse naming
table. Same as calling [get_type_pos] and extracting the path if the result
is a [Naming_types.TTypedef]. *)
val get_typedef_path : Provider_context.t -> string -> Relative_path.t option

(** Record that a class with the given name was declared at the given
position. Same as calling [add_type] with [Naming_types.TTypedef].
*)
val add_typedef : Provider_backend.t -> string -> FileInfo.pos -> unit

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit
