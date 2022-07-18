(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Aast
open Hh_prelude

let is_expr_same ((_, _, expr1) : Tast.expr) ((_, _, expr2) : Tast.expr) : bool
    =
  match (expr1, expr2) with
  | (Aast.True, Aast.True)
  | (Aast.False, Aast.False) ->
    true
  | (Aast.Int s1, Aast.Int s2)
  | (Aast.Float s1, Aast.Float s2)
  | (Aast.String s1, Aast.String s2)
  | (Aast.Id (_, s1), Aast.Id (_, s2))
    when String.equal s1 s2 ->
    true
  | ( Aast.Class_const ((_, _, Aast.CI (_, enum1)), (_, name1)),
      Aast.Class_const ((_, _, Aast.CI (_, enum2)), (_, name2)) )
    when String.equal enum1 enum2 && String.equal name1 name2 ->
    true
  | _ -> false

let rec are_return_expressions_same (ret_list : Tast.expr list) : bool =
  match ret_list with
  | expr1 :: expr2 :: tail ->
    if is_expr_same expr1 expr2 then
      are_return_expressions_same (expr2 :: tail)
    else
      false
  | [_] -> true
  | [] -> true

(* This function returns a list of all the expressions that follow each of the return statements in a block *)
let get_return_expr_visitor =
  object
    inherit [_] Aast.reduce as super

    method zero = []

    method plus = ( @ )

    method! on_stmt env s =
      match snd s with
      | Aast.Return (Some expr) -> [expr]
      | _ -> super#on_stmt env s

    (* we ignore the return expressions within lambda statements *)
    method! on_expr_ env e =
      match e with
      | Aast.Efun _ -> []
      | Aast.Lfun _ -> []
      | _ -> super#on_expr_ env e
  end

let get_return_exprs (stmts : Tast.stmt list) : Tast.expr list =
  get_return_expr_visitor#on_block () stmts

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def _env f =
      let ret_list = get_return_exprs f.fd_fun.f_body.fb_ast in
      if List.length ret_list > 1 then (
        if are_return_expressions_same ret_list then
          List.iter ret_list ~f:(fun (_, pos, _) ->
              Lints_errors.branches_return_same_value pos)
      ) else
        ()

    method! at_method_ _env m =
      let ret_list = get_return_exprs m.m_body.fb_ast in
      if List.length ret_list > 1 then (
        if are_return_expressions_same ret_list then
          List.iter ret_list ~f:(fun (_, pos, _) ->
              Lints_errors.branches_return_same_value pos)
      ) else
        ()
  end
