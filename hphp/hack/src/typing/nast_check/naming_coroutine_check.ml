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
      | Await _ when is_sync env -> Errors.await_in_sync_function p
      | _ -> ()

    method! at_stmt env =
      function
      | (_, Using { us_has_await; us_expr = (p, _); _ })
        when us_has_await && is_sync env ->
        Errors.await_in_sync_function p
      | (_, Foreach (_, (Await_as_v (p, _) | Await_as_kv (p, _, _)), _))
      | (p, Awaitall _)
        when is_sync env ->
        Errors.await_in_sync_function p
      | (p, Return (Some _)) when is_generator env -> Errors.return_in_gen p
      | _ -> ()
  end
