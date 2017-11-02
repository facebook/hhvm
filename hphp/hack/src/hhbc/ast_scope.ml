(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open Hh_core

module ScopeItem =
struct
  type is_static = bool
  type t =
  (* Named class *)
  | Class of Ast.class_
  (* Named function *)
  | Function of Ast.fun_
  (* Method in class *)
  | Method of Ast.method_
  (* PHP-style closure *)
  | LongLambda of is_static
  (* Short lambda *)
  | Lambda
end

module Scope =
struct
  type t = ScopeItem.t list
  let toplevel:t = []
  let rec get_class scope =
    match scope with
    | [] -> None
    | ScopeItem.Class cd :: _ -> Some cd
    | _ :: scope -> get_class scope

  let rec get_tparams scope =
    match scope with
    | [] -> []
    | ScopeItem.Class cd :: scope -> cd.Ast.c_tparams @ get_tparams scope
    | ScopeItem.Function fd :: scope -> fd.Ast.f_tparams @ get_tparams scope
    | ScopeItem.Method md :: scope -> md.Ast.m_tparams @ get_tparams scope
    | _ :: scope -> get_tparams scope

  let rec has_this scope =
    match scope with
    | [] -> true (* Assume top level has this *)
    | ScopeItem.Lambda :: scope
    | ScopeItem.LongLambda _ :: scope -> has_this scope
    | ScopeItem.Class _ :: _ -> false
    | ScopeItem.Function _ :: _ -> false
    | ScopeItem.Method _ :: _ -> true

  let is_toplevel scope = scope = []

  let rec is_in_static_method scope =
    match scope with
    | ScopeItem.Method md :: _ -> List.mem md.Ast.m_kind Ast.Static
    | ScopeItem.Lambda :: scope -> is_in_static_method scope
    | ScopeItem.LongLambda is_static :: scope ->
      not is_static && is_in_static_method scope
    | _ -> false

  let is_in_trait scope =
    match get_class scope with
    | None -> false
    | Some cd -> cd.Ast.c_kind = Ast.Ctrait
end
