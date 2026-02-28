(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type outline = string SymbolDefinition.t list

val summarize_property :
  string -> ('a, 'b) Aast.class_var -> Relative_path.t SymbolDefinition.t

val summarize_class_const :
  string -> ('a, 'b) Aast.class_const -> Relative_path.t SymbolDefinition.t

val summarize_class_typeconst :
  string ->
  ('a, 'b) Aast.class_typeconst_def ->
  Relative_path.t SymbolDefinition.t

val summarize_method :
  string -> ('a, 'b) Aast.method_ -> Relative_path.t SymbolDefinition.t

val summarize_class :
  ('a, 'b) Aast.class_ -> no_children:bool -> Relative_path.t SymbolDefinition.t

val summarize_typedef :
  ('a, 'b) Aast.typedef -> Relative_path.t SymbolDefinition.t

val summarize_fun : ('a, 'b) Aast.fun_def -> Relative_path.t SymbolDefinition.t

val summarize_gconst :
  ('a, 'b) Aast.gconst -> Relative_path.t SymbolDefinition.t

val summarize_local : string -> 'a Pos.pos -> 'a SymbolDefinition.t

val summarize_module_def :
  ('a, 'b) Aast.module_def -> Relative_path.t SymbolDefinition.t

val outline : ParserOptions.t -> string -> string SymbolDefinition.t list

val outline_entry_no_comments :
  popt:ParserOptions.t ->
  entry:Provider_context.entry ->
  string SymbolDefinition.t list

val print_def : ?short_pos:bool -> string -> string SymbolDefinition.t -> unit

val print : ?short_pos:bool -> string SymbolDefinition.t list -> unit
