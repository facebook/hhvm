(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val is_enum_or_enum_class : Ast_defs.classish_kind -> bool

(* True if source text ends in a newline *)
val ends_in_newline : Full_fidelity_source_text.t -> bool

val has_tabs_or_multibyte_codepoints : Full_fidelity_source_text.t -> bool

val get_type_from_hint : Provider_context.t -> Aast.hint -> string

val get_context_from_hint : Provider_context.t -> Aast.hint -> string

(* Values pulled from source code may have quotation marks;
strip these when present, eg: "\"FOO\"" => "FOO" *)
val strip_nested_quotes : string -> string

(* Convert ContainerName<TParam> to ContainerName *)
val strip_tparams : string -> string

(* Split name or subnamespace from its parent namespace, and return
either Some (parent, name), or None if the name has no parent namespace.
The trailing slash is removed from the parent. *)
val split_name : string -> (string * string) option

(* hack to pretty-print an expression. Get the representation from
  the source file, in lack of a better solution. This assumes that the
   expr comes from the the source text parameter. Should be replaced
   by proper pretty-printing functions. *)
val ast_expr_to_json :
  Full_fidelity_source_text.t -> ('a, 'b) Aast.expr -> Hh_json.json

val ast_expr_to_string :
  Full_fidelity_source_text.t -> ('a, 'b) Aast.expr -> string

exception Ast_error

exception Empty_namespace

(* Retrieve a namespace identifier and its position from an AST namespace node.
   Raise Ast_error if the ast doesn't have the expected structure, and Empty_namespace
   if the namespace is empty. ) *)
val namespace_ast_to_pos_id :
  Full_fidelity_positioned_syntax.syntax ->
  Full_fidelity_source_text.t ->
  Pos.t * string
