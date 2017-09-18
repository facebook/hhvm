(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open Core

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
    | ScopeItem.Lambda :: scope -> has_this scope
    | ScopeItem.LongLambda is_static :: scope ->
      not is_static && has_this scope
    | ScopeItem.Class _ :: _ -> false
    | ScopeItem.Function _ :: _ -> false
    | ScopeItem.Method md :: _ -> not (List.mem md.Ast.m_kind Ast.Static)

  let is_toplevel scope = scope = []
end
