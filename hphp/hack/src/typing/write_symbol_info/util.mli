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

exception Ast_error

exception Empty_namespace

(* Retrieve a namespace identifier and its position from an AST namespace node.
   Raise Ast_error if the ast doesn't have the expected structure, and Empty_namespace
   if the namespace is empty. ) *)
val namespace_ast_to_pos_id :
  Full_fidelity_positioned_syntax.syntax ->
  Full_fidelity_source_text.t ->
  Pos.t * string

(* remove generated parameters of the form T/[ctx $f]*)
val remove_generated_tparams :
  ('a, 'b) Aast_defs.tparam list -> ('a, 'b) Aast_defs.tparam list

val make_namespaceqname : string -> Hack.NamespaceQName.t

val make_name : string -> Hack.Name.t

val make_qname : string -> Hack.QName.t

val make_constraint_kind : Ast_defs.constraint_kind -> Hack.ConstraintKind.t

val make_visibility : Aast.visibility -> Hack.Visibility.t

val make_type_const_kind : Aast.class_typeconst -> Hack.TypeConstKind.t

val make_byte_span : 'a Pos.pos -> Src.ByteSpan.t

val make_variance : Ast_defs.variance -> Hack.Variance.t

val make_reify_kind : Ast_defs.reify_kind -> Hack.ReifyKind.t
