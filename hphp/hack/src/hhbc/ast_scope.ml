(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel

module T = Tast

module ScopeItem =
struct
  type is_static = bool
  type is_async = bool
  type rx_level = Rx.t option
  type t =
  (* Named class *)
  | Class of Ast.class_
  (* Named function *)
  | Function of Ast.fun_
  (* Method in class *)
  | Method of Ast.method_
  (* PHP-style closure *)
  | LongLambda of is_static * is_async * rx_level
  (* Short lambda *)
  | Lambda of is_async * rx_level

  type t_tast =
  | Class_tast of Tast.class_
  | Function_tast of Tast.fun_
  | Method_tast of Tast.method_
  | LongLambda_tast of is_static * is_async * rx_level
  | Lambda_tast of is_async * rx_level
end

module Scope =
struct
  type t = ScopeItem.t list
  type t_tast = ScopeItem.t_tast list
  let toplevel:t = []
  let toplevel_tast : t_tast = []
  let rec get_class scope =
    match scope with
    | [] -> None
    | ScopeItem.Class cd :: _ -> Some cd
    | _ :: scope -> get_class scope

  let rec get_class_tast scope =
    match scope with
    | [] -> None
    | ScopeItem.Class_tast cd :: _ -> Some cd
    | _ :: scope -> get_class_tast scope

  let rec get_span scope =
    match scope with
    | [] -> Pos.none
    | ScopeItem.Class cd :: _ -> cd.Ast.c_span
    | ScopeItem.Function fd :: _ -> fd.Ast.f_span
    | ScopeItem.Method md :: _ -> md.Ast.m_span
    | _ :: scope' -> get_span scope'

  let rec get_span_tast scope =
    match scope with
    | [] -> Pos.none
    | ScopeItem.Class_tast cd :: _ -> cd.T.c_span
    | ScopeItem.Function_tast fd :: _ -> fd.T.f_span
    | ScopeItem.Method_tast md :: _ -> md.T.m_span
    | _ :: scope' -> get_span_tast scope'

  let rec get_tparams scope =
    match scope with
    | [] -> []
    | ScopeItem.Class cd :: scope -> cd.Ast.c_tparams @ get_tparams scope
    | ScopeItem.Function fd :: scope -> fd.Ast.f_tparams @ get_tparams scope
    | ScopeItem.Method md :: scope -> md.Ast.m_tparams @ get_tparams scope
    | _ :: scope -> get_tparams scope

  let rec get_tparams_tast scope =
    match scope with
    | [] -> []
    | ScopeItem.Class_tast cd :: scope -> cd.T.c_tparams.T.c_tparam_list @ get_tparams_tast scope
    | ScopeItem.Function_tast fd :: scope -> fd.T.f_tparams @ get_tparams_tast scope
    | ScopeItem.Method_tast md :: scope -> md.T.m_tparams @ get_tparams_tast scope
    | _ :: scope -> get_tparams_tast scope

  let rec get_fun_tparams scope =
    match scope with
    | [] | ScopeItem.Class _ :: _ -> []
    | ScopeItem.Function fd :: _ -> fd.Ast.f_tparams
    | ScopeItem.Method md :: _ -> md.Ast.m_tparams
    | _ :: scope -> get_fun_tparams scope

  let rec get_fun_tparams_tast scope =
    match scope with
    | [] | ScopeItem.Class_tast _ :: _ -> []
    | ScopeItem.Function_tast fd :: _ -> fd.T.f_tparams
    | ScopeItem.Method_tast md :: _ -> md.T.m_tparams
    | _ :: scope -> get_fun_tparams_tast scope

  let rec get_class_tparams scope =
    match scope with
    | [] -> []
    | ScopeItem.Class cd :: _ -> cd.Ast.c_tparams
    | ScopeItem.Function _ :: scope
    | ScopeItem.Method _ :: scope
    | _ :: scope -> get_class_tparams scope

  let rec get_class_tparams_tast scope : T.class_tparams =
    match scope with
    | [] -> { T.c_tparam_list = []; T.c_tparam_constraints = SMap.empty }
    | ScopeItem.Class_tast cd :: _ -> cd.T.c_tparams
    | ScopeItem.Function_tast _ :: scope
    | ScopeItem.Method_tast _ :: scope
    | _ :: scope -> get_class_tparams_tast scope

  let rec has_this scope =
    match scope with
    | [] -> true (* Assume top level has this *)
    | ScopeItem.Lambda _ :: scope
    | ScopeItem.LongLambda _ :: scope -> has_this scope
    | ScopeItem.Class _ :: _ -> false
    | ScopeItem.Function _ :: _ -> false
    | ScopeItem.Method _ :: _ -> true

  let rec has_this_tast scope =
    match scope with
    | [] -> true (* Assume top level has this *)
    | ScopeItem.Lambda_tast _ :: scope
    | ScopeItem.LongLambda_tast _ :: scope -> has_this_tast scope
    | ScopeItem.Class_tast _ :: _ -> false
    | ScopeItem.Function_tast _ :: _ -> false
    | ScopeItem.Method_tast _ :: _ -> true

  let rec is_in_async scope =
    match scope with
    | [] -> false
    | ScopeItem.Lambda _ :: scope
    | ScopeItem.LongLambda _ :: scope -> is_in_async scope
    | ScopeItem.Class _ :: _ -> false
    | ScopeItem.Function { Ast.f_fun_kind = kind; _ } :: _
    | ScopeItem.Method { Ast.m_fun_kind = kind; _ } :: _ ->
      kind = Ast.FAsync || kind = Ast.FAsyncGenerator

  let rec is_in_async_tast scope =
    match scope with
    | [] -> false
    | ScopeItem.Lambda_tast _ :: scope
    | ScopeItem.LongLambda_tast _ :: scope -> is_in_async_tast scope
    | ScopeItem.Class_tast _ :: _ -> false
    | ScopeItem.Function_tast { T.f_fun_kind = kind; _ } :: _
    | ScopeItem.Method_tast { T.m_fun_kind = kind; _ } :: _ ->
      kind = Ast.FAsync || kind = Ast.FAsyncGenerator

  let is_toplevel scope = scope = []

  let rec is_in_static_method scope =
    match scope with
    | ScopeItem.Method md :: _ -> List.mem ~equal:(=) md.Ast.m_kind Ast.Static
    | ScopeItem.Lambda _ :: scope -> is_in_static_method scope
    | ScopeItem.LongLambda (is_static, _, _) :: scope ->
      not is_static && is_in_static_method scope
    | _ -> false

  let rec is_in_static_method_tast scope =
    match scope with
    | ScopeItem.Method_tast md :: _ -> md.T.m_static
    | ScopeItem.Lambda_tast _ :: scope -> is_in_static_method_tast scope
    | ScopeItem.LongLambda_tast (is_static, _, _) :: scope ->
      not is_static && is_in_static_method_tast scope
    | _ -> false

  let is_in_trait scope =
    match get_class scope with
    | None -> false
    | Some cd -> cd.Ast.c_kind = Ast.Ctrait

  let is_in_lambda = function
    | ScopeItem.Lambda _ :: _ | ScopeItem.LongLambda _ :: _ -> true
    | _ -> false

  let is_in_lambda_tast = function
    | ScopeItem.Lambda_tast _ :: _ | ScopeItem.LongLambda_tast _ :: _ -> true
    | _ -> false

  let rec rx_of_scope_tast (scope : ScopeItem.t_tast list): Rx.t =
    match scope with
    | []
    | ScopeItem.Class_tast _ :: _ ->
      Rx.NonRx
    | ScopeItem.Lambda_tast (_, Some rx_level) :: _
    | ScopeItem.LongLambda_tast (_, _, Some rx_level) :: _ ->
      rx_level
    | ScopeItem.Lambda_tast (_, None) :: rest
    | ScopeItem.LongLambda_tast (_, _, None) :: rest ->
      rx_of_scope_tast rest
    | ScopeItem.Method_tast { Tast.m_user_attributes = attrs; _ } :: _
    | ScopeItem.Function_tast { Tast.f_user_attributes = attrs; _ } :: _ ->
      Rx.rx_level_from_ast_tast attrs |> Option.value ~default:Rx.NonRx

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
    | ScopeItem.Method { Ast.m_user_attributes = attrs; _ } :: _
    | ScopeItem.Function { Ast.f_user_attributes = attrs; _ } :: _ ->
      Rx.rx_level_from_ast attrs |> Option.value ~default:Rx.NonRx
end
