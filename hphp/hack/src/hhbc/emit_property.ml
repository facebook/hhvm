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
  | _ -> true

let from_ast cv_kind_list _type_hint (_, (_, cv_name), initial_value) =
  (* TODO: Deal with type hint *)
  (* TODO: Hack allows a property to be marked final, which is nonsensical.
  HHVM does not allow this.  Fix this in the Hack parser? *)
  let pid = Hhbc_id.Prop.from_ast_name cv_name in
  let is_private = Core.List.mem cv_kind_list Ast.Private in
  let is_protected = Core.List.mem cv_kind_list Ast.Protected in
  let is_public =
    List.mem cv_kind_list Ast.Public
    || (not is_private && not is_protected) in
  let is_static = Core.List.mem cv_kind_list Ast.Static in
  let initial_value, is_deep_init, initializer_instrs =
    match initial_value with
    | None -> None, false, None
    | Some expr ->
      match Ast_constant_folder.expr_to_opt_typed_value
        (Emit_expression.get_namespace ()) expr with
      | Some v ->
        Some v, false, None
      | None ->
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
        Some Typed_value.Uninit, not is_static && expr_requires_deep_init expr,
          Some (gather [
            prolog;
            Emit_expression.from_expr expr;
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
