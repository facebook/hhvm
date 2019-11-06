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
open Nast_check_env

let is_coroutine env =
  Option.equal
    Ast_defs.equal_fun_kind
    env.function_kind
    (Some Ast_defs.FCoroutine)

let is_generator env =
  let fun_kind = env.function_kind in
  Option.equal Ast_defs.equal_fun_kind fun_kind (Some Ast_defs.FGenerator)
  || Option.equal
       Ast_defs.equal_fun_kind
       fun_kind
       (Some Ast_defs.FAsyncGenerator)

let is_sync env =
  let fun_kind = env.function_kind in
  Option.equal Ast_defs.equal_fun_kind fun_kind (Some Ast_defs.FGenerator)
  || Option.equal Ast_defs.equal_fun_kind fun_kind (Some Ast_defs.FSync)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr env (p, e) =
      match e with
      | Yield _
      | Yield_break
      | Yield_from _ ->
        if is_coroutine env then Errors.yield_in_coroutine p
      | Await _ ->
        if is_coroutine env then Errors.await_in_coroutine p;
        if is_sync env then Errors.await_in_sync_function p
      | Suspend _ ->
        if not (is_coroutine env) then
          Errors.suspend_outside_of_coroutine p
        else if env.is_finally then
          Errors.suspend_in_finally p
      | _ -> ()

    method! at_stmt env s =
      match snd s with
      | Using { us_has_await = has_await; us_expr = e; _ } when has_await ->
        let p = fst e in
        if is_coroutine env then Errors.await_in_coroutine p;
        if is_sync env then Errors.await_in_sync_function p
      | Foreach (_, (Await_as_v (p, _) | Await_as_kv (p, _, _)), _) ->
        if is_coroutine env then Errors.await_in_coroutine p;
        if is_sync env then Errors.await_in_sync_function p
      | Return (Some _) when is_generator env -> Errors.return_in_gen (fst s)
      | Awaitall _ ->
        let p = fst s in
        if is_coroutine env then Errors.await_in_coroutine p;
        if is_sync env then Errors.await_in_sync_function p
      | _ -> ()
  end
