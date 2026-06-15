(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Aast
open Hh_prelude

(**
   Lint on functions/methods that always return the same value. This
   is usually a copy-paste error.

   function some_predicate(Whatever $value): bool {
     if ($value->foo()) {
       return false;
     }

     if ($value->bar()) {
       return false;
     }

     return false; // oops!
   } *)

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

(* Does this value look like a success exit code? We don't want to
   lint on code like this:

   function my_main(): int {
     if (quick_check()) { return 0; }

     normal_work();
     return 0;
   } *)
let is_success_ish ((_, _, e_) : Tast.expr) : bool =
  match e_ with
  | Aast.Int "0" -> true
  | Aast.Class_const (_, (_, "SUCCESS")) -> true
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

type return_or_throw =
  | Return_expr of Tast.expr
  | Throw_stmt

(* This function returns all return expressions and throw statements in a
   block. *)
let get_return_or_throw_visitor =
  object
    inherit [_] Aast.reduce as super

    method zero = []

    method plus = ( @ )

    method! on_stmt env s =
      match snd s with
      | Aast.Return (Some expr) -> [Return_expr expr]
      | Aast.Throw _ -> [Throw_stmt]
      | _ -> super#on_stmt env s

    (* We ignore return expressions and throw statements within lambdas. *)
    method! on_expr_ env e =
      match e with
      | Aast.Efun _ -> []
      | Aast.Lfun _ -> []
      | _ -> super#on_expr_ env e
  end

let get_return_or_throw (stmts : Tast.stmt list) : return_or_throw list =
  get_return_or_throw_visitor#on_block () stmts

let check_block (block : Tast.block) : unit =
  let return_or_throw = get_return_or_throw block in
  let has_throw =
    List.exists return_or_throw ~f:(function
        | Throw_stmt -> true
        | Return_expr _ -> false)
  in
  let ret_list =
    List.filter_map return_or_throw ~f:(function
        | Return_expr expr -> Some expr
        | Throw_stmt -> None)
  in
  match ret_list with
  | expr1 :: _ :: _ ->
    if
      (not has_throw)
      && are_return_expressions_same ret_list
      && not (is_success_ish expr1)
    then
      List.iter ret_list ~f:(fun (_, pos, _) ->
          Lints_diagnostics.branches_return_same_value pos)
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def _env f = check_block f.fd_fun.f_body.fb_ast

    method! at_method_ _env m = check_block m.m_body.fb_ast
  end
