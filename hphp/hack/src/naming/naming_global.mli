(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** GEnv is solely a set of thin wrappers around Naming_provider. *)
module GEnv : sig
  val get_fun_full_pos :
    Provider_context.t -> FileInfo.pos * string -> Pos.t * string

  val get_type_full_pos :
    Provider_context.t -> FileInfo.pos * string -> Pos.t * string

  val get_const_full_pos :
    Provider_context.t -> FileInfo.pos * string -> Pos.t * string

  val type_pos : Provider_context.t -> string -> Pos.t option

  val type_info :
    Provider_context.t -> string -> (Pos.t * Naming_types.kind_of_type) option

  val fun_pos : Provider_context.t -> string -> Pos.t option

  val typedef_pos : Provider_context.t -> string -> Pos.t option

  val gconst_pos : Provider_context.t -> string -> Pos.t option
end

(* Removing declarations *)
val remove_decls :
  backend:Provider_backend.t ->
  funs:string list ->
  classes:string list ->
  typedefs:string list ->
  consts:string list ->
  modules:string list ->
  unit

(* Same as remove_decls but extracts definition identifiers from the file_info *)
val remove_decls_using_file_info : Provider_backend.t -> FileInfo.t -> unit

(** This function "declares" top-level names, i.e. adds them into the naming-table provider
(which is a wrapper for the reverse naming table). It handles "name already bound" errors as follows:
- If any symbol from the FileInfo.t is already defined in a different file (with or without the
same case), that other file is deemed to have the "canonical" definition of the symbol; this
function leaves the naming-table-provider for the conflicting name as it is, and also returns the
canonical's filename plus this filename.
- If any symbol from the FileInfo.t is defined twice within the FileInfo.t (with or without
the same case), then the first occurrence is deemed to be the "canonical" definition of the symbol;
this function ends with the first occurrence in the naming-table-provider, and reports an error
on the second, and returns this filename.

There are expectations of the caller:
- This function doesn't touch the forward naming table; that's left to the caller.
- The caller is expected to ensure that all names from the specified file have already been removed
from the naming-table provider prior to calling this function.
- The caller is expected to provide "full" positions in its FileInfo.t.

TODO(ljw): This function always returns an empty Errors.t. It never produces errors.
All "already-bound" errors are produced during [Typing_toplevel]. *)
val ndecl_file_error_if_already_bound :
  Provider_context.t ->
  Relative_path.t ->
  FileInfo.t ->
  Errors.t * Relative_path.Set.t

(** This function "declares" top-level names, i.e. adds them into the naming-table provider.
This caller is expected to ensure that there are no naming-collisons and no case-insensitive naming collisions. *)
val ndecl_file_skip_if_already_bound :
  Provider_context.t -> Relative_path.t -> FileInfo.t -> unit
