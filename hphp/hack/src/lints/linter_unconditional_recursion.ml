(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Aast
open Hh_prelude

let same_name_ignoring_ns (name1 : string) (name2 : string) =
  String.equal (Utils.strip_ns name1) (Utils.strip_ns name2)

let check_expr_fun (expr_ : Tast.expr_) (fun_name : string) =
  match expr_ with
  | Aast.Call ((_, _, Aast.Id (_, name)), _, _, _) ->
    same_name_ignoring_ns name fun_name
  | _ -> false

let check_expr_method
    (expr_ : Tast.expr_) (method_name : string) (c_name : string) =
  match expr_ with
  | Aast.Call
      ( ( _,
          _,
          Aast.Obj_get
            ((_, _, Aast.This), (_, _, Aast.Id (_, name)), _, Aast.Is_method) ),
        _,
        _,
        _ ) ->
    same_name_ignoring_ns name method_name
  | Aast.Call
      ( (_, _, Aast.Class_const ((_, _, Aast.CI (_, name_of_class)), (_, name))),
        _,
        _,
        _ )
    when same_name_ignoring_ns name_of_class c_name ->
    same_name_ignoring_ns name method_name
  | Aast.Call
      ((_, _, Aast.Class_const ((_, _, Aast.CIstatic), (_, name))), _, _, _) ->
    same_name_ignoring_ns name method_name
  | Aast.Call
      ((_, _, Aast.Class_const ((_, _, Aast.CIself), (_, name))), _, _, _) ->
    same_name_ignoring_ns name method_name
  | _ -> false

let is_recursion_in_fun ((_, stmt_) : Tast.stmt) (fun_name : string) =
  match stmt_ with
  | Aast.Return (Some (_, _, Aast.Await (_, _, await_exp))) ->
    check_expr_fun await_exp fun_name
  | Aast.Return (Some (_, _, e)) -> check_expr_fun e fun_name
  | Aast.Expr (_ty, _pos, Aast.Await (_, _, await_exp)) ->
    check_expr_fun await_exp fun_name
  | Aast.Expr (_ty, _pos, expr_) -> check_expr_fun expr_ fun_name
  | _ -> false

let is_recursion_in_method
    ((_, stmt_) : Tast.stmt) (method_name : string) (c_name : string) =
  match stmt_ with
  | Aast.Return (Some (_, _, Aast.Await (_, _, await_exp))) ->
    check_expr_method await_exp method_name c_name
  | Aast.Return (Some (_, _, e)) -> check_expr_method e method_name c_name
  | Aast.Expr (_ty, _pos, Aast.Await (_, _, await_exp)) ->
    check_expr_method await_exp method_name c_name
  | Aast.Expr (_ty, _pos, expr_) -> check_expr_method expr_ method_name c_name
  | _ -> false

let check_unconditional_recursion m c_name =
  match m.m_body.fb_ast with
  | head :: _ ->
    if is_recursion_in_method head (snd m.m_name) c_name then
      Lints_errors.unconditional_recursion m.m_span
    else
      ()
  | [] -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def _env f =
      match f.fd_fun.f_body.fb_ast with
      | head :: _ ->
        if is_recursion_in_fun head (snd f.fd_fun.f_name) then
          Lints_errors.unconditional_recursion f.fd_fun.f_span
        else
          ()
      | [] -> ()

    method! at_class_ _env c =
      List.iter c.c_methods ~f:(fun c_method ->
          check_unconditional_recursion c_method (snd c.c_name))
  end
