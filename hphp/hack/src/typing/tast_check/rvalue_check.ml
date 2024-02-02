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

let check_valid_rvalue pos env ty =
  let rec iter_over_types env tyl =
    match tyl with
    | [] -> env
    | ty :: tyl ->
      let (env, ety) = Env.expand_type env ty in
      (match deref ety with
      | (r, Tprim Tnoreturn) ->
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            wellformedness
            @@ Primary.Wellformedness.Noreturn_usage
                 {
                   pos;
                   reason =
                     lazy
                       (Reason.to_string
                          "A `noreturn` function always throws or exits."
                          r);
                 });
        env
      | (r, Tprim Tvoid) ->
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            wellformedness
            @@ Primary.Wellformedness.Void_usage
                 {
                   pos;
                   reason =
                     lazy
                       (Reason.to_string
                          "A `void` function doesn't return a value."
                          r);
                 });
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

    method! on_expr env ((ty, p, e) as te) =
      match e with
      | Binop { bop = Ast_defs.Eq None; lhs; rhs } ->
        this#allow_non_returning (fun () -> this#on_expr env lhs);
        this#disallow_non_returning (fun () -> this#on_expr env rhs)
      | Eif (e1, e2, e3) ->
        this#disallow_non_returning (fun () -> this#on_expr env e1);
        Option.iter e2 ~f:(this#on_expr env);
        this#on_expr env e3
      | Pipe (_, e1, e2) ->
        this#disallow_non_returning (fun () -> this#on_expr env e1);
        this#on_expr env e2
      | List el -> List.iter el ~f:(this#on_expr env)
      | ReadonlyExpr r ->
        (* ReadonlyExprs can be immediately surrounding a void thing,
           but the thing inside the expression should be checked for void *)
        this#disallow_non_returning (fun () -> super#on_expr env r)
      | ExpressionTree
          {
            et_class;
            et_splices;
            et_function_pointers;
            et_virtualized_expr;
            et_runtime_expr;
            et_dollardollar_pos = _;
          } ->
        this#on_id env et_class;
        this#on_block env et_splices;
        this#on_block env et_function_pointers;
        (* Allow calls to void functions at the top level:

             Code`void_func()`

           but not in subexpressions:

             Code`() ==> { $x = void_func(); }`
        *)
        super#on_expr env et_virtualized_expr;

        this#on_expr env et_runtime_expr
      | _ ->
        if not !non_returning_allowed then check_valid_rvalue p env ty;
        this#disallow_non_returning (fun () -> super#on_expr env te)

    method! on_stmt env stmt =
      match snd stmt with
      | Expr e -> this#allow_non_returning (fun () -> this#on_expr env e)
      | Return (Some (_, _, Hole (e, _, _, _)))
      | Return (Some e) ->
        this#allow_non_returning (fun () -> this#on_expr env e)
      | For (e1, e2, e3, b) ->
        this#allow_non_returning (fun () -> List.iter ~f:(this#on_expr env) e1);
        this#disallow_non_returning (fun () ->
            Option.iter ~f:(this#on_expr env) e2);
        this#allow_non_returning (fun () -> List.iter ~f:(this#on_expr env) e3);
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
