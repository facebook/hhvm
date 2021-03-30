(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module "naming" a program.
 * Get all the global names
 *)

module GEnv : sig
  val get_fun_full_pos :
    Provider_context.t -> FileInfo.pos * string -> Pos.t * string

  val get_type_full_pos :
    Provider_context.t -> FileInfo.pos * string -> Pos.t * string

  val get_const_full_pos :
    Provider_context.t -> FileInfo.pos * string -> Pos.t * string

  val type_pos : Provider_context.t -> string -> Pos.t option

  val type_canon_name : Provider_context.t -> string -> string option

  val type_info :
    Provider_context.t -> string -> (Pos.t * Naming_types.kind_of_type) option

  val fun_pos : Provider_context.t -> string -> Pos.t option

  val fun_canon_name : Provider_context.t -> string -> string option

  val typedef_pos : Provider_context.t -> string -> Pos.t option

  val gconst_pos : Provider_context.t -> string -> Pos.t option
end

(* Removing declarations *)
val remove_decls :
  backend:Provider_backend.t ->
  funs:string list ->
  classes:string list ->
  record_defs:string list ->
  typedefs:string list ->
  consts:string list ->
  unit

(** This function "declares" top-level names, i.e. adds
them into the naming-table provider. The only checks done
here are whether an entity name was already in the naming-table (e.g. when
two files define the same function, up to case insensitivity). *)
val ndecl_file_error_if_already_bound :
  Provider_context.t ->
  Relative_path.t ->
  FileInfo.t ->
  Errors.t * Relative_path.Set.t

(** This function "declares" top-level names, i.e. adds them into
the naming-table provider. If a name was already bound (up to case
insensitivy) then this function just skips them, trusting that
they're already okay, or that we wouldn't even be called in that
situation in first place. *)
val ndecl_file_skip_if_already_bound :
  Provider_context.t -> Relative_path.t -> FileInfo.t -> unit
