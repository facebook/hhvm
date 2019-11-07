(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val find_class_in_file :
  ?full:bool ->
  ?case_insensitive:bool ->
  Relative_path.t ->
  string ->
  Nast.class_ option

val find_record_def_in_file :
  ?full:bool ->
  ?case_insensitive:bool ->
  Relative_path.t ->
  string ->
  Nast.record_def option

val find_fun_in_file :
  ?full:bool ->
  ?case_insensitive:bool ->
  Relative_path.t ->
  string ->
  Nast.fun_ option

val find_typedef_in_file :
  ?full:bool ->
  ?case_insensitive:bool ->
  Relative_path.t ->
  string ->
  Nast.typedef option

val find_gconst_in_file :
  ?full:bool -> Relative_path.t -> string -> Nast.gconst option

val get_ast : ?full:bool -> Relative_path.t -> Nast.program

val parse_file_input :
  ?full:bool ->
  Relative_path.t ->
  ServerCommandTypes.file_input ->
  Full_fidelity_source_text.t * Nast.program

val local_changes_push_stack : unit -> unit

val local_changes_pop_stack : unit -> unit

val local_changes_commit_batch : Relative_path.Set.t -> unit

val local_changes_revert_batch : Relative_path.Set.t -> unit

type parse_type =
  | Decl
  | Full

val provide_ast_hint : Relative_path.t -> Nast.program -> parse_type -> unit

val remove_batch : Relative_path.Set.t -> unit

val has_for_test : Relative_path.t -> bool
