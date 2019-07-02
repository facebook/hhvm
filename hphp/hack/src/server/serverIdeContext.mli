(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t
(** A context mapping from file to set of symbols contained in that file. Can be
applied using [with_context] to load the declarations of those symbols while
calling a function, and then unloading those symbols afterward. *)

type entry
(** The information associated with a given file. *)

val empty: t
(** The empty context mapping. *)

val update:
  tcopt:TypecheckerOptions.t ->
  ctx:t ->
  path:Relative_path.t ->
  file_input:ServerCommandTypes.file_input ->
  (t * entry)
(** Parse and typecheck the given AST for the given path, and return an updated
[t] containing that entry. *)

val with_context: ctx:t -> f:(unit -> 'a) -> 'a
(** Load the declarations of [t] and call [f], then unload those declarations.
*)

val get_file_input:
  ctx:t ->
  path:Relative_path.t ->
  ServerCommandTypes.file_input
(** Returns a [ServerCommandTypes.file_input] corresponding to the given [path].

If the [path] is in the context, returns its associated
[ServerCommandTypes.FileContent]. Otherwise returns the
[ServerCommandTypes.FileName] corresponding to that file on disk. *)

val get_path: entry:entry -> Relative_path.t
(** Get the path associated with the given [entry]. *)

val get_ast: entry:entry -> Ast.program
(** Get the AST associated with the given [entry]. *)

val get_tast: entry:entry -> Tast.program
(** Get the typed AST associated with the given [entry]. *)

val get_fileinfo: entry:entry -> FileInfo.t
(** Get the FileInfo.t associated with the given [entry]. *)
