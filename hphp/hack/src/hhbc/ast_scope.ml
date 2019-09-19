(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
module T = Aast

module ScopeItem = struct
  type is_static = bool

  type is_async = bool

  type rx_level = Rx.t option

  type t =
    (* Named class *)
    | Class of Tast.class_
    (* Named function *)
    | Function of Tast.fun_
    (* Method in class *)
    | Method of Tast.method_
    (* PHP-style closure *)
    | LongLambda of is_static * is_async * rx_level
    (* Short lambda *)
    | Lambda of is_async * rx_level
end

module Scope = struct
  type t = ScopeItem.t list

  let toplevel : t = []

  let rec get_class scope =
    match scope with
    | [] -> None
    | ScopeItem.Class cd :: _ -> Some cd
    | _ :: scope -> get_class scope

  let rec get_span scope =
    match scope with
    | [] -> Pos.none
    | ScopeItem.Class cd :: _ -> cd.T.c_span
    | ScopeItem.Function fd :: _ -> fd.T.f_span
    | ScopeItem.Method md :: _ -> md.T.m_span
    | _ :: scope' -> get_span scope'

  let rec get_tparams scope =
    match scope with
    | [] -> []
    | ScopeItem.Class cd :: scope ->
      cd.T.c_tparams.T.c_tparam_list @ get_tparams scope
    | ScopeItem.Function fd :: scope -> fd.T.f_tparams @ get_tparams scope
    | ScopeItem.Method md :: scope -> md.T.m_tparams @ get_tparams scope
    | _ :: scope -> get_tparams scope

  let rec get_fun_tparams scope =
    match scope with
    | []
    | ScopeItem.Class _ :: _ ->
      []
    | ScopeItem.Function fd :: _ -> fd.T.f_tparams
    | ScopeItem.Method md :: _ -> md.T.m_tparams
    | _ :: scope -> get_fun_tparams scope

  let rec get_class_tparams scope =
    match scope with
    | [] -> { T.c_tparam_list = []; T.c_tparam_constraints = SMap.empty }
    | ScopeItem.Class cd :: _ -> cd.T.c_tparams
    | ScopeItem.Function _ :: scope
    | ScopeItem.Method _ :: scope
    | _ :: scope ->
      get_class_tparams scope

  let rec has_this scope =
    match scope with
    | [] -> true (* Assume top level has this *)
    | ScopeItem.Lambda _ :: scope
    | ScopeItem.LongLambda _ :: scope ->
      has_this scope
    | ScopeItem.Class _ :: _ -> false
    | ScopeItem.Function _ :: _ -> false
    | ScopeItem.Method _ :: _ -> true

  let rec is_in_async scope =
    match scope with
    | [] -> false
    | ScopeItem.Lambda _ :: scope
    | ScopeItem.LongLambda _ :: scope ->
      is_in_async scope
    | ScopeItem.Class _ :: _ -> false
    | ScopeItem.Function { T.f_fun_kind = kind; _ } :: _
    | ScopeItem.Method { T.m_fun_kind = kind; _ } :: _ ->
      kind = Ast_defs.FAsync || kind = Ast_defs.FAsyncGenerator

  let is_toplevel scope = scope = []

  let rec is_in_static_method scope =
    match scope with
    | ScopeItem.Method md :: _ -> md.T.m_static
    | ScopeItem.Lambda _ :: scope -> is_in_static_method scope
    | ScopeItem.LongLambda (is_static, _, _) :: scope ->
      (not is_static) && is_in_static_method scope
    | _ -> false

  let is_in_trait scope =
    match get_class scope with
    | None -> false
    | Some cd -> cd.T.c_kind = Ast_defs.Ctrait

  let is_in_lambda = function
    | ScopeItem.Lambda _ :: _
    | ScopeItem.LongLambda _ :: _ ->
      true
    | _ -> false

  let rec rx_of_scope scope =
    match scope with
    | []
    | ScopeItem.Class _ :: _ ->
      Rx.NonRx
    | ScopeItem.Lambda (_, Some rx_level) :: _
    | ScopeItem.LongLambda (_, _, Some rx_level) :: _ ->
      rx_level
    | ScopeItem.Lambda (_, None) :: rest
    | ScopeItem.LongLambda (_, _, None) :: rest ->
      rx_of_scope rest
    | ScopeItem.Method { T.m_user_attributes = attrs; _ } :: _
    | ScopeItem.Function { T.f_user_attributes = attrs; _ } :: _ ->
      Rx.rx_level_from_ast attrs |> Option.value ~default:Rx.NonRx

  let rec find_enclosing_function scope =
    match scope with
    | [] -> None
    | (ScopeItem.Function _ as f) :: _ -> Some f
    | (ScopeItem.Method _ as m) :: _ -> Some m
    | _ :: more -> find_enclosing_function more

  let find_function_attribute scope attr_name =
    let find_attribute ua_list =
      List.find ua_list (fun attr -> snd attr.T.ua_name = attr_name)
    in
    match find_enclosing_function scope with
    | Some (ScopeItem.Function f) -> find_attribute f.T.f_user_attributes
    | Some (ScopeItem.Method m) -> find_attribute m.T.m_user_attributes
    | _ -> None

  let has_function_attribute scope attr_name =
    Option.is_some @@ find_function_attribute scope attr_name
end
