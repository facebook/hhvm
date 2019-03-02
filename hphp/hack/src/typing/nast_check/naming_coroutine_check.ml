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
  fun_kind: Ast.fun_kind;
  is_finally: bool;
}

let set_fun_kind ctx f =
  { ctx with fun_kind = f.f_fun_kind }

let is_coroutine ctx =
  ctx.fun_kind = Ast.FCoroutine

let is_generator ctx =
  ctx.fun_kind = Ast.FGenerator || ctx.fun_kind = Ast.FAsyncGenerator
let default_ctx =
  {
    is_finally = false;
    fun_kind = Ast.FSync;
  }

let visitor = object(self)
  inherit [ctx] Nast_visitor.iter_with_state as super

  method! on_fun_ (env, ctx) f =
    super#on_fun_ (env, set_fun_kind ctx f) f

  method! on_expr (env, ctx) (p, e) =
    begin match e with
    | Yield _
    | Yield_break
    | Yield_from _ ->
      if is_coroutine ctx then Errors.yield_in_coroutine p
    | Await _ ->
      if is_coroutine ctx then Errors.await_in_coroutine p
    | Suspend _ ->
      if not (is_coroutine ctx) then Errors.suspend_outside_of_coroutine p
      else if ctx.is_finally then Errors.suspend_in_finally p
    | _ -> ()
    end;
    super#on_expr (env, ctx) (p, e)

  method! on_stmt (env, ctx) s =
    begin match s with
    | Using { us_has_await = has_await; us_expr = e;  _ } when is_coroutine ctx ->
      if has_await then Errors.await_in_coroutine (fst e)
    | Foreach (_, (Await_as_v (p, _) | Await_as_kv (p, _, _)), _)
      when is_coroutine ctx ->
        Errors.await_in_coroutine p
    | Try (_, _, fb) when is_coroutine ctx ->
      self#on_block (env, { ctx with is_finally = true }) fb
    | Return (p, Some _) when is_generator ctx ->
      Errors.return_in_gen p
    | _ -> ()
    end;
    super#on_stmt (env, ctx) s

end

let handler = object
  inherit Nast_visitor.handler_base

  method! at_fun_ env f = visitor#on_fun_ (env, set_fun_kind default_ctx f) f

end
