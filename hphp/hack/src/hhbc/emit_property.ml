(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Instruction_sequence
open Hhbc_ast
open Core
module A = Ast

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
  | _ -> true

and expr_pair_requires_deep_init (expr1, expr2) =
  expr_requires_deep_init expr1 || expr_requires_deep_init expr2

and aexpr_requires_deep_init aexpr =
  match aexpr with
  | A.AFvalue expr -> expr_requires_deep_init expr
  | A.AFkvalue (expr1, expr2) ->
    expr_requires_deep_init expr1 || expr_requires_deep_init expr2

let from_ast ast_class cv_kind_list type_hint tparams namespace
             (_, (pos, cv_name), initial_value) =
  (* TODO: Hack allows a property to be marked final, which is nonsensical.
  HHVM does not allow this.  Fix this in the Hack parser? *)
  let pid = Hhbc_id.Prop.from_ast_name cv_name in
  let is_private = Core.List.mem cv_kind_list Ast.Private in
  let is_protected = Core.List.mem cv_kind_list Ast.Protected in
  let is_public =
    List.mem cv_kind_list Ast.Public
    || (not is_private && not is_protected) in
  let is_static = Core.List.mem cv_kind_list Ast.Static in
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
      Emit_type_hint.(hint_to_type_info
        ~kind:Property ~nullable:false
        ~skipawaitable:false ~tparams ~namespace h) in
  let env = Emit_env.make_class_env ast_class in
  let initial_value, is_deep_init, initializer_instrs =
    match initial_value with
    | None -> None, false, None
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
        Some v, false, None
      | _ ->
        let label = Label.next_regular () in
        let prolog, epilog =
          if is_static
          then empty, instr (IMutator (InitProp (pid, Static)))
          else if is_private
          then empty, instr (IMutator (InitProp (pid, NonStatic)))
          else
            gather [
              instr (IMutator (CheckProp pid));
              instr_jmpnz label;
            ],
            gather [
              instr (IMutator (InitProp (pid, NonStatic)));
              instr_label label;
            ] in
        Some Typed_value.Uninit, deep_init,
          Some (gather [
            prolog;
            Emit_expression.emit_expr ~need_ref:false env expr;
            epilog]) in
  Hhas_property.make
    is_private
    is_protected
    is_public
    is_static
    is_deep_init
    false (*no_serialize*)
    pid
    initial_value
    initializer_instrs
    tinfo
