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

(** Removing defs from reverse naming table *)
val remove_decls :
  backend:Provider_backend.t ->
  funs:string list ->
  classes:string list ->
  typedefs:string list ->
  consts:string list ->
  modules:string list ->
  unit

(* Same as remove_decls but extracts definition identifiers from the file_info *)
val remove_decls_using_file_info : Provider_backend.t -> FileInfo.ids -> unit

(** This function "declares" top-level names, i.e. adds them into the naming-table provider
(which is a wrapper for the reverse naming table). As for duplicate name definitions, they
are a joint responsibility between the reverse-naming-table and [env.failed_naming];
this function is responsible for maintaining the invariant...
- The invariant is that if there are duplicate names, then [env.failed_naming] contains all
filenames that declare those duplicate names (both winners and losers).
- If any symbol from the FileInfo.t is already defined in a different file (with or without the
same case), that other file is deemed to have the "winner" definition of the symbol; this
function leaves the reverse-naming-table for the conflicting name as it is, and also returns the
winner's filename plus this filename.
- If any symbol from the FileInfo.t is defined twice within the FileInfo.t (with or without
the same case), then the first occurrence is deemed to be the "winner" definition of the symbol;
this function ends with the first occurrence in the reverse-naming-table, and returns this filename.
- Actually, the way serverTypeCheck works is that whenever it is asked to do a typecheck, then
it already first calls [Naming_global.remove_decls] on ALL [env.failed_naming] in addition to
all modified files, and then calls this function [Naming_global.ndecl_file_and_get_conflict_files]
on the same list of files. In this way, despite the precisely phrased bullet points above,
the net effect of serverTypeCheck is that the winners end up being defined just by the order
in which files get sent to this function, which is in fact [Relative_path.Map.fold], which ends
up as an implementation detail being defined by [Relative_path.compare].

Returns:
- This function returns a set of all "conflict files" discovered:
if you call [ndecl_file_and_get_conflict_files ctx fn file_info], and no symbol in [file_info]
conflicts with any existing entries in the reverse naming table, then it returns an empty set;
but for all symbols that do conflict then it returns [fn] as well as the filename that defines
the winner for that symbol.

There are expectations of the caller:
- This function doesn't touch the forward naming table; that's left to the caller.
- The caller is expected to ensure that all names from the specified file have already been removed
from the naming-table provider prior to calling this function.
- The caller is expected to provide "full" positions in its FileInfo.t. *)
val ndecl_file_and_get_conflict_files :
  Provider_context.t -> Relative_path.t -> FileInfo.ids -> Relative_path.Set.t

(** This function "declares" top-level names, i.e. adds them into the naming-table provider.
This caller is expected to ensure that there are no naming-collisons and no case-insensitive naming collisions. *)
val ndecl_file_skip_if_already_bound :
  Provider_context.t -> Relative_path.t -> FileInfo.ids -> unit
