(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hhbc_ast
module A = Aast
module SN = Naming_special_names
module SU = Hhbc_string_utils

(* Class expressions; similar to Nast.class_id *)
type class_expr =
  | Class_special of SpecialClsRef.t
  | Class_id of Ast_defs.id
  | Class_expr of Tast.expr
  | Class_reified of Instruction_sequence.t

let get_original_class_name ~resolve_self ~check_traits scope =
  match Ast_scope.Scope.get_class scope with
  | None -> None
  | Some cd ->
    if (cd.A.c_kind = Ast_defs.Ctrait && not check_traits) || not resolve_self
    then
      None
    else
      let class_name = snd cd.A.c_name in
      (* If we're in a closure we unmangle the name to get the original class *)
      (match SU.Closures.unmangle_closure class_name with
      | None -> Some class_name
      | Some _ when resolve_self ->
        begin
          match
            SMap.find_opt class_name (Emit_env.get_closure_enclosing_classes ())
          with
          | Some cd when cd.Emit_env.kind <> Ast_defs.Ctrait ->
            Some cd.Emit_env.name
          | _ -> None
        end
      | _ -> None)

let get_parent_class_name cd =
  match cd.A.c_extends with
  | [(_, A.Happly ((_, parent_cid), _))] -> Some parent_cid
  | _ -> None

let get_original_parent_class_name ~check_traits ~resolve_self scope =
  match Ast_scope.Scope.get_class scope with
  | None -> None
  | Some cd ->
    (* Parent doesn't make sense on an interface, so we treat
      "parent" as the literal class id to grab from *)
    if cd.A.c_kind = Ast_defs.Cinterface then
      Some SN.Classes.cParent
    else if
      (cd.A.c_kind = Ast_defs.Ctrait && not check_traits) || not resolve_self
    then
      None
    else
      let class_name = snd cd.A.c_name in
      (match SU.Closures.unmangle_closure class_name with
      | Some _ ->
        let cd_opt =
          SMap.find_opt class_name (Emit_env.get_closure_enclosing_classes ())
        in
        Option.bind cd_opt (fun x -> x.Emit_env.parent_class_name)
      | None -> get_parent_class_name cd)

let expr_to_class_expr
    ?(check_traits = false) ~resolve_self scope ((_, expr_) as expr) =
  match expr_ with
  | A.Id (_, id) when SU.is_static id -> Class_special SpecialClsRef.Static
  | A.Id (pos, id) when SU.is_parent id ->
    begin
      match
        get_original_parent_class_name ~resolve_self ~check_traits scope
      with
      | Some name -> Class_id (pos, name)
      | None -> Class_special SpecialClsRef.Parent
    end
  | A.Id (pos, id) when SU.is_self id ->
    begin
      match get_original_class_name ~resolve_self ~check_traits scope with
      | Some name -> Class_id (pos, name)
      | None -> Class_special SpecialClsRef.Self
    end
  | A.Id id -> Class_id id
  | _ -> Class_expr expr

(* The legacy AST used expr node everywhere where the AAST uses a class_id node.
 * The ast_to_nast module translates all of these occurrences to CIexpr nodes
 * so always expect CIexpr as the class_id node
 *)
let class_id_to_class_expr
    ?(check_traits = false) ~resolve_self scope (cid : Tast.class_id) :
    class_expr =
  let (annot, cid_) = cid in
  let e =
    match cid_ with
    | A.CIexpr e -> e
    | A.CI sid -> (annot, A.Id sid)
    | _ -> failwith "TODO Unimplemented class_id_to_class_expr"
  in
  expr_to_class_expr ~check_traits ~resolve_self scope e
