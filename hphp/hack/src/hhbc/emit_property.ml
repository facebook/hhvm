(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Instruction_sequence
open Hhbc_ast
open Hhbc_string_utils
open Hh_core
module A = Ast
module SN = Naming_special_names
module SU = Hhbc_string_utils
module TC = Hhas_type_constraint

(* Follow HHVM rules here: see EmitterVisitor::requiresDeepInit *)
let rec expr_requires_deep_init (_, expr_) =
  match expr_ with
  | A.Unop((A.Uplus | A.Uminus), e1) ->
    expr_requires_deep_init e1
  | A.Binop(_, e1, e2) ->
    expr_requires_deep_init e1 || expr_requires_deep_init e2
  | A.Lvar _ | A.Null | A.False | A.True | A.Int _
  | A.Float _ | A.String _ -> false
  | A.Array fields | A.Collection ((_, ("keyset" | "vec" | "dict")), fields) ->
    List.exists fields aexpr_requires_deep_init
  | A.Varray fields -> List.exists fields expr_requires_deep_init
  | A.Darray fields -> List.exists fields expr_pair_requires_deep_init
  | A.Id(_, ("__FILE__" | "__DIR__")) -> false
  | A.Class_const ((_, A.Id (_, s)), (_, p)) ->
    class_const_requires_deep_init s p
  | A.Shape fields ->
    List.exists fields shape_field_requires_deep_init
  | _ -> true

and class_const_requires_deep_init s p =
  not (SU.is_class p) ||
  SU.is_self s ||
  SU.is_parent s ||
  SU.is_static s

and shape_field_requires_deep_init (n, v) =
  match n with
  | A.SFlit_int _ | A.SFlit_str _ ->
    expr_requires_deep_init v
  | A.SFclass_const ((_, s), (_, p)) ->
    class_const_requires_deep_init s p ||
    expr_requires_deep_init v

and expr_pair_requires_deep_init (expr1, expr2) =
  expr_requires_deep_init expr1 || expr_requires_deep_init expr2

and aexpr_requires_deep_init aexpr =
  match aexpr with
  | A.AFvalue expr -> expr_requires_deep_init expr
  | A.AFkvalue (expr1, expr2) ->
    expr_requires_deep_init expr1 || expr_requires_deep_init expr2

let valid_tc_for_prop tc =
  match (TC.name tc) with
  | None -> true
  | Some name ->
     not (is_self name) &&
     not (is_parent name) &&
     not (String.lowercase_ascii name = "callable") &&
     not (String.lowercase_ascii name = "hh\\noreturn")

let from_ast
    ast_class
    cv_user_attributes
    cv_kind_list
    class_is_immutable
    type_hint
    tparams
    namespace
    doc_comment_opt
    (_, (pos, cv_name), initial_value) =
  (* TODO: Hack allows a property to be marked final, which is nonsensical.
  HHVM does not allow this.  Fix this in the Hack parser? *)
  let pid = Hhbc_id.Prop.from_ast_name cv_name in
  let attributes = Emit_attribute.from_asts namespace cv_user_attributes in
  let is_immutable = class_is_immutable ||
    Hhas_attribute.has_const attributes in
  let is_lsb = Hhas_attribute.has_lsb attributes in
  let is_private = Hh_core.List.mem cv_kind_list Ast.Private in
  let is_protected = Hh_core.List.mem cv_kind_list Ast.Protected in
  let is_public =
    List.mem cv_kind_list Ast.Public
    || (not is_private && not is_protected) in
  let is_static = Hh_core.List.mem cv_kind_list Ast.Static in
  if not is_static
    && ast_class.Ast.c_final
    && ast_class.Ast.c_kind = Ast.Cabstract
  then Emit_fatal.raise_fatal_parse pos
    ("Class " ^ Utils.strip_ns (snd ast_class.Ast.c_name) ^
      " contains non-static property declaration"
     ^ " and therefore cannot be declared 'abstract final'");
  let tinfo =
    match type_hint with
    | None ->
      Hhas_type_info.make (Some "") (Hhas_type_constraint.make None [])
    | Some h ->
       let tc =
         Emit_type_hint.(hint_to_type_info
                         ~kind:Property ~nullable:false
                         ~skipawaitable:false ~tparams ~namespace h)
       in
       if not (valid_tc_for_prop (Hhas_type_info.type_constraint tc))
       then Emit_fatal.raise_fatal_parse pos
         (Printf.sprintf "Invalid property type hint for '%s::$%s'"
                         (Utils.strip_ns (snd ast_class.Ast.c_name))
                         (Hhbc_id.Prop.to_raw_string pid))
       else tc
  in
  let env = Emit_env.make_class_env ast_class in
  let initial_value, is_deep_init, has_system_initial, initializer_instrs =
    match initial_value with
    | None -> None, false, true, None
    | Some expr ->
      let is_collection_map =
        match snd expr with
        | A.Collection ((_, ("Map" | "ImmMap")), _) -> true
        | _ -> false
      in
      let deep_init = not is_static && expr_requires_deep_init expr in
      match Ast_constant_folder.expr_to_opt_typed_value
        ast_class.Ast.c_namespace expr with
      | Some v when not deep_init && not is_collection_map ->
        Some v, false, false, None
      | _ ->
        let label = Label.next_regular () in
        let prolog, epilog =
          if is_static
          then empty, Emit_pos.emit_pos_then ast_class.Ast.c_span @@
            instr (IMutator (InitProp (pid, Static)))
          else if is_private
          then empty, Emit_pos.emit_pos_then ast_class.Ast.c_span @@
            instr (IMutator (InitProp (pid, NonStatic)))
          else
            gather [
              Emit_pos.emit_pos ast_class.Ast.c_span;
              instr (IMutator (CheckProp pid));
              instr_jmpnz label;
            ],
            gather [
              Emit_pos.emit_pos ast_class.Ast.c_span;
              instr (IMutator (InitProp (pid, NonStatic)));
              instr_label label;
            ] in
        Some Typed_value.Uninit, deep_init, false,
          Some (gather [
            prolog;
            Emit_expression.emit_expr ~need_ref:false env expr;
            epilog]) in
  Hhas_property.make
    attributes
    is_private
    is_protected
    is_public
    is_static
    is_deep_init
    false (*no_serialize*)
    is_immutable
    is_lsb
    false (*no_bad_redeclare*)
    has_system_initial
    false (*no_implicit_null*)
    false (*initial_satisfies_tc*)
    pid
    initial_value
    initializer_instrs
    tinfo
    doc_comment_opt
