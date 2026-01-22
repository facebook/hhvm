(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Case-sensitive.
    `full` determines whether this is producing the full AST or just the decl information. *)
val find_class_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.class_ option

(** Case-insensitive *)
val find_iclass_in_file :
  Provider_context.t -> Relative_path.t -> string -> Nast.class_ option

(** Case-sensitive.
    `full` determines whether this is producing the full AST or just the decl information. *)
val find_fun_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.fun_def option

(** Case-insensitive *)
val find_ifun_in_file :
  Provider_context.t -> Relative_path.t -> string -> Nast.fun_def option

(** Case-sensitive.
    `full` determines whether this is producing the full AST or just the decl information. *)
val find_typedef_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.typedef option

(** Case-insensitive *)
val find_itypedef_in_file :
  Provider_context.t -> Relative_path.t -> string -> Nast.typedef option

(** Case-sensitive.
    `full` determines whether this is producing the full AST or just the decl information. *)
val find_gconst_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.gconst option

(** Case-sensitive.
    `full` determines whether this is producing the full AST or just the decl information. *)
val find_module_in_file :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  string ->
  Nast.module_def option

(** `full` determines whether this is producing the full AST or just the decl information. *)
val get_ast : full:bool -> Provider_context.t -> Relative_path.t -> Nast.program

(** `full` determines whether this is producing the full AST or just the decl information. *)
val get_ast_with_error :
  full:bool ->
  Provider_context.t ->
  Relative_path.t ->
  Diagnostics.t * Nast.program

(** Compute the AST for the given [Provider_context.entry].
    This is cached in the entry itself, but not in the AST provider's cache. *)
val compute_ast :
  popt:ParserOptions.t -> entry:Provider_context.entry -> Nast.program

(** Compute the full [Parser_return.t] object.
    This is cached in the entry itself, but not in the AST provider's cache. *)
val compute_parser_return_and_ast_errors :
  popt:ParserOptions.t ->
  entry:Provider_context.entry ->
  Parser_return.t * Diagnostics.t

(** Compute the comments for the given [Provider_context.entry].
    This is cached in the entry itself, but not in the AST provider's cache. *)
val compute_comments :
  popt:ParserOptions.t -> entry:Provider_context.entry -> Parser_return.comments

(** Compute the [FileInfo.t] associated with the given entry, doing a parse
if necessary.
    This is cached in the entry itself, but not in the AST provider's cache. *)
val compute_file_info :
  popt:ParserOptions.t -> entry:Provider_context.entry -> FileInfo.ids

(** Compute the [Full_fidelity_source_text.t] for this [Provider_context.entry].
    This is cached in the entry itself, but not in the AST provider's cache. *)
val compute_source_text :
  entry:Provider_context.entry -> Full_fidelity_source_text.t

(** Compute the concrete syntax tree for this [Provider_context.entry].
    This is cached in the entry itself, but not in the AST provider's cache. *)
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
