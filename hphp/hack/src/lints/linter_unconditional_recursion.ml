(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Aast
open Hh_prelude

type method_prop = {
  method_name: string;
  class_name: string;
  pos: Pos.t;
}

type fun_prop = {
  fun_name: string;
  pos: Pos.t;
}

type is_fun_or_method =
  | Is_fun of fun_prop
  | Is_method of method_prop

let same_name_ignoring_ns (name1 : string) (name2 : string) =
  String.equal (Utils.strip_ns name1) (Utils.strip_ns name2)

let check_expr (expr_ : Tast.expr_) (is_fun_or_method : is_fun_or_method) =
  match is_fun_or_method with
  | Is_fun f_prop ->
    (match expr_ with
    | Aast.Call { func = (_, _, Aast.Id (_, name)); _ } ->
      same_name_ignoring_ns name f_prop.fun_name
    | _ -> false)
  | Is_method m_prop ->
    (match expr_ with
    | Aast.Call
        {
          func =
            ( _,
              _,
              Aast.Obj_get
                ((_, _, Aast.This), (_, _, Aast.Id (_, name)), _, Aast.Is_method)
            );
          _;
        } ->
      same_name_ignoring_ns name m_prop.method_name
    | Aast.Call
        {
          func =
            ( _,
              _,
              Aast.Class_const ((_, _, Aast.CI (_, name_of_class)), (_, name))
            );
          _;
        }
      when same_name_ignoring_ns name_of_class m_prop.class_name ->
      same_name_ignoring_ns name m_prop.method_name
    | Aast.Call
        {
          func = (_, _, Aast.Class_const ((_, _, Aast.CIstatic), (_, name)));
          _;
        } ->
      same_name_ignoring_ns name m_prop.method_name
    | Aast.Call
        { func = (_, _, Aast.Class_const ((_, _, Aast.CIself), (_, name))); _ }
      ->
      same_name_ignoring_ns name m_prop.method_name
    | _ -> false)

let is_recursive_call
    ((_, stmt_) : Tast.stmt) (is_fun_or_method : is_fun_or_method) =
  match stmt_ with
  | Aast.Return (Some (_, _, Aast.Await (_, _, await_exp))) ->
    check_expr await_exp is_fun_or_method
  | Aast.Return (Some (_, _, e)) -> check_expr e is_fun_or_method
  | Aast.Throw (_ty, _pos, expr_) -> check_expr expr_ is_fun_or_method
  | Aast.Expr (_ty, _pos, Aast.Await (_, _, await_exp)) ->
    check_expr await_exp is_fun_or_method
  | Aast.Expr (_ty, _pos, expr_) -> check_expr expr_ is_fun_or_method
  | _ -> false

(* This function returns true if the recursion can be terminated. This occurs when we find at least one return or throw statment that is followed by any expression other than a recursive call*)
let can_terminate_visitor is_fun_or_method =
  object
    inherit [_] Aast.reduce as super

    method zero = false

    method plus = ( || )

    method! on_stmt env s =
      match snd s with
      | Aast.Return _ -> not (is_recursive_call s is_fun_or_method)
      | Aast.Throw _ -> not (is_recursive_call s is_fun_or_method)
      | _ -> super#on_stmt env s
  end

let can_terminate (s : Tast.stmt) (is_fun_or_method : is_fun_or_method) : bool =
  (can_terminate_visitor is_fun_or_method)#on_stmt () s

let rec check_unconditional_recursion
    (stmt_list : Tast.stmt list) (is_fun_or_method : is_fun_or_method) =
  match stmt_list with
  | head :: tail ->
    if can_terminate head is_fun_or_method then
      ()
    else if is_recursive_call head is_fun_or_method then
      match is_fun_or_method with
      | Is_fun f_prop -> Lints_errors.unconditional_recursion f_prop.pos
      | Is_method m_prop -> Lints_errors.unconditional_recursion m_prop.pos
    else
      check_unconditional_recursion tail is_fun_or_method
  | [] -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def _env f =
      let f_properties : fun_prop =
        { fun_name = snd f.fd_name; pos = f.fd_fun.f_span }
      in
      let is_f_or_m : is_fun_or_method = Is_fun f_properties in
      check_unconditional_recursion f.fd_fun.f_body.fb_ast is_f_or_m

    method! at_class_ _env c =
      List.iter c.c_methods ~f:(fun c_method ->
          let m_properties : method_prop =
            {
              method_name = snd c_method.m_name;
              class_name = snd c.c_name;
              pos = c_method.m_span;
            }
          in
          let is_f_or_m : is_fun_or_method = Is_method m_properties in
          check_unconditional_recursion c_method.m_body.fb_ast is_f_or_m)
  end
