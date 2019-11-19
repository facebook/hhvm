(*
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
open Core_kernel
module A = Ast_defs
module T = Aast
module SN = Naming_special_names
module SU = Hhbc_string_utils
module TC = Hhas_type_constraint

(* Follow HHVM rules here: see EmitterVisitor::requiresDeepInit *)
let rec expr_requires_deep_init (_, expr_) =
  match expr_ with
  | T.Unop ((A.Uplus | A.Uminus), e1) -> expr_requires_deep_init e1
  | T.Binop (_, e1, e2) ->
    expr_requires_deep_init e1 || expr_requires_deep_init e2
  | T.Lvar _
  | T.Null
  | T.False
  | T.True
  | T.Int _
  | T.Float _
  | T.String _ ->
    false
  | T.Array fields
  | T.Collection ((_, ("keyset" | "vec" | "dict")), _, fields) ->
    List.exists fields aexpr_requires_deep_init
  | T.Varray (_, fields) -> List.exists fields expr_requires_deep_init
  | T.Darray (_, fields) -> List.exists fields expr_pair_requires_deep_init
  | T.Id (_, name)
    when name = SN.PseudoConsts.g__FILE__ || name = SN.PseudoConsts.g__DIR__ ->
    false
  | T.Class_const ((_, T.CIexpr (_, T.Id (_, s))), (_, p)) ->
    class_const_requires_deep_init s p
  | T.Shape fields -> List.exists fields shape_field_requires_deep_init
  | T.Typename _
  | T.Assert _
  | T.Pair _
  | T.Smethod_id _
  | T.Method_caller _
  | T.Method_id _
  | T.Fun_id _
  | T.Dollardollar _
  | T.Lplaceholder _
  | T.This
  | T.KeyValCollection _
  | T.ValCollection _
  | T.Any ->
    failwith "Expr not found on original AST"
  | _ -> true

and class_const_requires_deep_init s p =
  (not (SU.is_class p)) || SU.is_self s || SU.is_parent s || SU.is_static s

and shape_field_requires_deep_init (n, v) =
  match n with
  | A.SFlit_int _
  | A.SFlit_str _ ->
    expr_requires_deep_init v
  | A.SFclass_const ((_, s), (_, p)) ->
    class_const_requires_deep_init s p || expr_requires_deep_init v

and expr_pair_requires_deep_init (expr1, expr2) =
  expr_requires_deep_init expr1 || expr_requires_deep_init expr2

and aexpr_requires_deep_init aexpr =
  match aexpr with
  | T.AFvalue expr -> expr_requires_deep_init expr
  | T.AFkvalue (expr1, expr2) ->
    expr_requires_deep_init expr1 || expr_requires_deep_init expr2

let valid_tc_for_prop tc =
  match TC.name tc with
  | None -> true
  | Some name ->
    (not (is_self name))
    && (not (is_parent name))
    && (not (String.lowercase name = "callable"))
    && (not (String.lowercase name = "hh\\nothing"))
    && not (String.lowercase name = "hh\\noreturn")

let from_ast
    class_
    cv_user_attributes
    is_abstract
    is_static
    cv_visibility
    class_is_const
    type_hint
    tparams
    namespace
    doc_comment_opt
    (_, (pos, cv_name), initial_value) =
  (* TODO: Hack allows a property to be marked final, which is nonsensical.
  HHVM does not allow this.  Fix this in the Hack parser? *)
  let pid = Hhbc_id.Prop.from_ast_name cv_name in
  let attributes = Emit_attribute.from_asts namespace cv_user_attributes in
  let is_const =
    ((not is_static) && class_is_const) || Hhas_attribute.has_const attributes
  in
  let is_lsb = Hhas_attribute.has_lsb attributes in
  let is_late_init = Hhas_attribute.has_late_init attributes in
  let visibility = cv_visibility in
  let is_private = cv_visibility = Aast.Private in
  if (not is_static) && class_.T.c_final && class_.T.c_kind = Ast_defs.Cabstract
  then
    Emit_fatal.raise_fatal_parse
      pos
      ( "Class "
      ^ Utils.strip_ns (snd class_.T.c_name)
      ^ " contains non-static property declaration"
      ^ " and therefore cannot be declared 'abstract final'" );
  let tinfo =
    match type_hint with
    | None -> Hhas_type_info.make (Some "") (Hhas_type_constraint.make None [])
    | Some h ->
      let tc =
        Emit_type_hint.hint_to_type_info
          ~kind:Emit_type_hint.Property
          ~nullable:false
          ~skipawaitable:false
          ~tparams
          h
      in
      if not (valid_tc_for_prop (Hhas_type_info.type_constraint tc)) then
        Emit_fatal.raise_fatal_parse
          pos
          (Printf.sprintf
             "Invalid property type hint for '%s::$%s'"
             (Utils.strip_ns (snd class_.T.c_name))
             (Hhbc_id.Prop.to_raw_string pid))
      else
        tc
  in
  let env = Emit_env.make_class_env class_ in
  let (initial_value, is_deep_init, has_system_initial, initializer_instrs) =
    match initial_value with
    | None ->
      let v =
        if is_late_init then
          Some Typed_value.Uninit
        else
          None
      in
      (v, false, true, None)
    | Some expr ->
      if is_late_init then
        Emit_fatal.raise_fatal_parse
          pos
          (Printf.sprintf
             "<<__LateInit>> property '%s::$%s' cannot have an initial value"
             (Utils.strip_ns (snd class_.T.c_name))
             (Hhbc_id.Prop.to_raw_string pid));
      let is_collection_map =
        match snd expr with
        | T.Collection ((_, ("Map" | "ImmMap")), _, _) -> true
        | _ -> false
      in
      let deep_init = (not is_static) && expr_requires_deep_init expr in
      let otv =
        Ast_constant_folder.expr_to_opt_typed_value class_.T.c_namespace expr
      in
      (match otv with
      | Some v when (not deep_init) && not is_collection_map ->
        (Some v, false, false, None)
      | _ ->
        let label = Label.next_regular () in
        let (prolog, epilog) =
          if is_static then
            ( empty,
              Emit_pos.emit_pos_then class_.T.c_span
              @@ instr (IMutator (InitProp (pid, Static))) )
          else if is_private then
            ( empty,
              Emit_pos.emit_pos_then class_.T.c_span
              @@ instr (IMutator (InitProp (pid, NonStatic))) )
          else
            ( gather
                [
                  Emit_pos.emit_pos class_.T.c_span;
                  instr (IMutator (CheckProp pid));
                  instr_jmpnz label;
                ],
              gather
                [
                  Emit_pos.emit_pos class_.T.c_span;
                  instr (IMutator (InitProp (pid, NonStatic)));
                  instr_label label;
                ] )
        in
        ( Some Typed_value.Uninit,
          deep_init,
          false,
          Some (gather [prolog; Emit_expression.emit_expr env expr; epilog]) ))
  in
  Hhas_property.make
    attributes
    visibility
    is_abstract
    is_static
    is_deep_init
    is_const
    is_lsb
    false (*no_bad_redeclare*)
    has_system_initial
    false (*no_implicit_null*)
    false (*initial_satisfies_tc*)
    is_late_init
    pid
    initial_value
    initializer_instrs
    tinfo
    doc_comment_opt
