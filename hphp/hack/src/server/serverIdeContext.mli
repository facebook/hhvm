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
  ast:Ast.program ->
  (t * entry)
(** Parse and typecheck the given AST for the given path, and return an updated
[t] containing that entry. *)

val with_context: ctx:t -> f:(unit -> 'a) -> 'a
(** Load the declarations of [t] and call [f], then unload those declarations.
*)

val get_ast: entry:entry -> Ast.program
val get_tast: entry:entry -> Tast.program
