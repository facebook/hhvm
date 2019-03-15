(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


val make_local_changes: unit -> unit
val revert_local_changes: unit -> unit
(* When typechecking a content buffer in IDE mode,
* this is the path that will be assigned to it *)
val path: Relative_path.t

(** Runs the declaration, naming, and typecheck phases on a single file. *)
val check_file_input :
  TypecheckerOptions.t ->
  (* What are the definitions in each file. *)
  Naming_table.t ->
  (* File path or content buffer. When given file path, the mapping in file_info
   * is used to find the declarations and run only the typechecking phase.
   * When given content buffer it will be parsed, named, and declared before
   * that. The declarations will be removed from shared memory afterwards. *)
  ServerCommandTypes.file_input ->
  Relative_path.t * Tast.program

val check_fileinfo :
  TypecheckerOptions.t ->
  Relative_path.t ->
  FileInfo.t ->
  Tast.program

(** Runs the declaration, naming, and typecheck phases on an already-parsed
    AST. *)
val check_ast :
  TypecheckerOptions.t ->
  Ast.program ->
  Tast.program

(* Parses, names, declares and typechecks the content buffer, then run f
 * while the declared definitions are still available in shared memory.
 * The declarations will be removed from shared memory afterwards. *)
val declare_and_check : string ->
  f:(Relative_path.t -> FileInfo.t -> Tast.program -> 'a) -> TypecheckerOptions.t -> 'a

val get_errors: Relative_path.t ->  string -> TypecheckerOptions.t -> Errors.t

(* Run the typing phase on a list of files and definitions they contain. *)
val recheck :
  TypecheckerOptions.t ->
  (Relative_path.t * FileInfo.t) list ->
  (Relative_path.t * Tast.program) list
