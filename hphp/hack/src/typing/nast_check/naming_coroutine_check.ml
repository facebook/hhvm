(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast

module SN = Naming_special_names

type ctx = {
  is_coroutine: bool;
  is_async: bool;
  is_final: bool;
}

let set_ctx ctx f =
  match f.f_fun_kind with
  | Ast.FCoroutine ->
    { ctx with is_coroutine = true }
  | Ast.FSync
  | Ast.FAsync
  | Ast.FGenerator
  | Ast.FAsyncGenerator ->
    { ctx with is_async = true }

let default_ctx = {
  is_coroutine = false;
  is_async = false;
  is_final = false;
}

let visitor = object(self)
  inherit [ctx] Nast_visitor.iter_with_state as super

  method! on_fun_ (env, ctx) f =
    super#on_fun_ (env, set_ctx ctx f) f

  method! on_expr (env, ctx) (p, e) =
    match e with
    | Yield _
    | Yield_break
    | Yield_from _ ->
      if ctx.is_coroutine then Errors.yield_in_coroutine p
    | Await _ ->
      if ctx.is_coroutine then Errors.await_in_coroutine p
    | Suspend _ ->
      if ctx.is_coroutine && ctx.is_final then Errors.suspend_in_finally p;
      if ctx.is_async && not ctx.is_coroutine then Errors.suspend_outside_of_coroutine p
    | _ -> ();
    super#on_expr (env, ctx) (p, e)

  method! on_stmt (env, ctx) s =
    if ctx.is_coroutine then
    begin match s with
    | Using { us_has_await = has_await; us_expr = e;  _ } ->
      if has_await then Errors.await_in_coroutine (fst e)
    | Foreach (_, (Await_as_v (p, _) | Await_as_kv (p, _, _)), _) ->
      Errors.await_in_coroutine p
    | Try (_, _, fb) ->
      self#on_block (env, { ctx with is_final = true }) fb
    | _ -> ()
    end;
    super#on_stmt (env, ctx) s

end

let handler = object
  inherit Nast_visitor.handler_base

  method! at_expr env e = visitor#on_expr (env, default_ctx) e

  method! at_stmt env s = visitor#on_stmt (env, default_ctx) s

  method! at_fun_ env f = visitor#on_fun_ (env, set_ctx default_ctx f) f

end
