(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val find_class_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.class_ option

val find_iclass_in_file :
  Provider_context.t -> Relative_path.t -> string -> Nast.class_ option

val find_fun_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.fun_def option

val find_ifun_in_file :
  Provider_context.t -> Relative_path.t -> string -> Nast.fun_def option

val find_typedef_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.typedef option

val find_itypedef_in_file :
  Provider_context.t -> Relative_path.t -> string -> Nast.typedef option

val find_gconst_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.gconst option

val find_module_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.module_def option

val get_ast : full:bool -> Provider_context.t -> Relative_path.t -> Nast.program

val get_ast_with_error :
  full:bool -> Provider_context.t -> Relative_path.t -> Errors.t * Nast.program

(** Compute the AST for the given [Provider_context.entry]. *)
val compute_ast :
  popt:ParserOptions.t -> entry:Provider_context.entry -> Nast.program

(** Compute the full [Parser_return.t] object. *)
val compute_parser_return_and_ast_errors :
  popt:ParserOptions.t ->
  entry:Provider_context.entry ->
  Parser_return.t * Errors.t

(** Compute the comments for the given [Provider_context.entry]. *)
val compute_comments :
  popt:ParserOptions.t -> entry:Provider_context.entry -> Parser_return.comments

(** Compute the [FileInfo.t] associated with the given entry, doing a parse
if necessary. *)
val compute_file_info :
  popt:ParserOptions.t -> entry:Provider_context.entry -> FileInfo.ids

(** Compute the [Full_fidelity_source_text.t] for this [Provider_context.entry]. *)
val compute_source_text :
  entry:Provider_context.entry -> Full_fidelity_source_text.t

(** Compute the concrete syntax tree for this [Provider_context.entry]. *)
val compute_cst :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  Provider_context.PositionedSyntaxTree.t

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit

type parse_type =
  | Decl
  | Full

val provide_ast_hint : Relative_path.t -> Nast.program -> parse_type -> unit

val remove_batch : Relative_path.Set.t -> unit

val has_for_test : Relative_path.t -> bool

val clear_parser_cache : unit -> unit

val clear_local_cache : unit -> unit
