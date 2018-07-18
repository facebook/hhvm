(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

module A = Ast
module SN = Naming_special_names
module SU = Hhbc_string_utils

(* Class expressions; similar to Nast.class_id *)
type class_expr =
| Class_parent
| Class_self
| Class_static
| Class_id of Ast.id
| Class_expr of Ast.expr
| Class_unnamed_local of Local.t

let get_original_class_name ~resolve_self ~check_traits scope =
  match Ast_scope.Scope.get_class scope with
  | None ->  None
  | Some cd ->
    if (cd.Ast.c_kind = Ast.Ctrait && not check_traits) || not resolve_self
    then None
    else
    let class_name = snd cd.Ast.c_name in
    (* If we're in a closure we unmangle the name to get the original class *)
    match SU.Closures.unmangle_closure class_name with
    | None -> Some class_name
    | Some _ when resolve_self ->
      begin match SMap.get class_name (Emit_env.get_closure_enclosing_classes ()) with
      | Some cd when cd.Ast.c_kind <> Ast.Ctrait -> Some (snd cd.Ast.c_name)
      | _ -> None
      end
    | _ -> None

let get_parent_class_name cd =
  match cd.A.c_extends with
  | [(_, A.Happly((_, parent_cid), _))] -> Some parent_cid
  | _ -> None

let get_original_parent_class_name ~check_traits ~resolve_self scope =
  match Ast_scope.Scope.get_class scope with
  | None -> None
  | Some cd ->
    (* Parent doesn't make sense on an interface, so we treat
      "parent" as the literal class id to grab from *)
    if cd.Ast.c_kind = Ast.Cinterface then Some SN.Classes.cParent else
    if (cd.Ast.c_kind = Ast.Ctrait && not check_traits)
      || not resolve_self
    then None
    else
    let class_name = snd cd.Ast.c_name in
    match SU.Closures.unmangle_closure class_name with
    | Some _ ->
      let cd_opt = SMap.get class_name (Emit_env.get_closure_enclosing_classes ()) in
      Option.bind cd_opt get_parent_class_name
    | None -> get_parent_class_name cd

(* Return true in second component if this is a forwarding reference *)
let expr_to_class_expr ?(check_traits=false) ~resolve_self scope (_, expr_ as expr) =
  match expr_ with
  | A.Id (_, id) when SU.is_static id -> Class_static
  | A.Id (pos, id) when SU.is_parent id ->
    begin match get_original_parent_class_name ~resolve_self ~check_traits scope with
    | Some name -> Class_id (pos, name)
    | None -> Class_parent
    end
  | A.Id (pos, id) when SU.is_self id ->
    begin match get_original_class_name ~resolve_self ~check_traits scope with
    | Some name -> Class_id (pos, name)
    | None -> Class_self
    end
  | A.Id id -> Class_id id
  | _ -> Class_expr expr
