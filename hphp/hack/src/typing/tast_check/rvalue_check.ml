(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module Reason = Typing_reason

let check_valid_rvalue p env ty =
  let rec iter_over_types env tyl =
    match tyl with
    | [] -> env
    | ty :: tyl ->
      let (env, ety) = Env.expand_type env ty in
      (match deref ety with
      | (r, Tprim Tnoreturn) ->
        Errors.noreturn_usage
          p
          (Reason.to_string "A `noreturn` function always throws or exits." r);
        env
      | (r, Tprim Tvoid) ->
        Errors.void_usage
          p
          (Reason.to_string "A `void` function doesn't return a value." r);
        env
      | (_, Tunion tyl2) -> iter_over_types env (tyl2 @ tyl)
      | (_, _) -> iter_over_types env tyl)
  in
  ignore (iter_over_types env [ty])

let visitor =
  object (this)
    inherit [_] Aast.iter as super

    val non_returning_allowed = ref true

    method allow_non_returning f =
      let is_non_returning_allowed = !non_returning_allowed in
      non_returning_allowed := true;
      f ();
      non_returning_allowed := is_non_returning_allowed

    method disallow_non_returning f =
      let is_non_returning_allowed = !non_returning_allowed in
      non_returning_allowed := false;
      f ();
      non_returning_allowed := is_non_returning_allowed

    method! on_expr env (((p, ty), e) as te) =
      match e with
      | Binop (Ast_defs.Eq None, e1, e2) ->
        this#allow_non_returning (fun () -> this#on_expr env e1);
        this#disallow_non_returning (fun () -> this#on_expr env e2)
      | Eif (e1, e2, e3) ->
        this#disallow_non_returning (fun () -> this#on_expr env e1);
        Option.iter e2 (this#on_expr env);
        this#on_expr env e3
      | Pipe (_, e1, e2) ->
        this#disallow_non_returning (fun () -> this#on_expr env e1);
        this#on_expr env e2
      | List el -> List.iter el (this#on_expr env)
      | Expr_list el -> List.iter el (this#on_expr env)
      | Suspend e -> this#allow_non_returning (fun () -> this#on_expr env e)
      | _ ->
        if not !non_returning_allowed then check_valid_rvalue p env ty;
        this#disallow_non_returning (fun () -> super#on_expr env te)

    method! on_stmt env stmt =
      match snd stmt with
      | Expr e -> this#allow_non_returning (fun () -> this#on_expr env e)
      | Return (Some e) ->
        this#allow_non_returning (fun () -> this#on_expr env e)
      | For (e1, e2, e3, b) ->
        this#allow_non_returning (fun () -> this#on_expr env e1);
        this#disallow_non_returning (fun () -> this#on_expr env e2);
        this#allow_non_returning (fun () -> this#on_expr env e3);
        this#on_block env b
      | Foreach (e1, e2, b) ->
        this#disallow_non_returning (fun () -> this#on_expr env e1);
        this#allow_non_returning (fun () -> this#on_as_expr env e2);
        this#on_block env b
      | Awaitall _ ->
        this#allow_non_returning (fun () -> super#on_stmt env stmt)
      | _ -> this#disallow_non_returning (fun () -> super#on_stmt env stmt)

    method! on_block env block =
      this#allow_non_returning (fun () -> super#on_block env block)
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt = visitor#on_stmt
  end
