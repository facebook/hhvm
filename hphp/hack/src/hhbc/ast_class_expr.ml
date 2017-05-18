(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

let get_original_class_name scope =
  match Ast_scope.Scope.get_class scope with
  | None ->  None
  | Some cd ->
    if cd.Ast.c_kind = Ast.Ctrait
    then None
    else
    (* If we're in a closure we unmangle the name to get the original class *)
    match SU.Closures.unmangle_closure (snd cd.Ast.c_name) with
    | None -> Some (snd cd.Ast.c_name)
    | Some scope_name ->
      match SU.Closures.split_scope_name scope_name with
      | [class_name] -> Some class_name
      | [class_name; _method_name] -> Some class_name
      | _ -> None

let get_original_parent_class_name scope =
  match Ast_scope.Scope.get_class scope with
  | None -> None
  | Some cd ->
    match SU.Closures.unmangle_closure (snd cd.Ast.c_name) with
    | Some _ -> None
    | None ->
      match cd.A.c_extends with
      | [(_, A.Happly((_, parent_cid), _))] -> Some parent_cid
      | _ -> None

(* Return true in second component if this is a forwarding reference *)
let expr_to_class_expr scope (_, expr_ as expr) =
  match expr_ with
  | A.Id (_, id) when id = SN.Classes.cStatic ->
    Class_static, false
  | A.Id (pos, id) when id = SN.Classes.cParent ->
    begin match get_original_parent_class_name scope with
    | Some name -> Class_id (pos, name), true
    | None -> Class_parent, true
    end
  | A.Id (pos, id) when id = SN.Classes.cSelf ->
    begin match get_original_class_name scope with
    | Some name -> Class_id (pos, name), true
    | None -> Class_self, true
    end
  | A.Id id | A.Id_type_arguments (id, _) ->
    Class_id id, false
  | _ ->
    Class_expr expr, false

let id_to_expr (pos, name as id) =
  if name.[0] = '$'
  then (pos, A.Lvar id)
  else (pos, A.Id id)
