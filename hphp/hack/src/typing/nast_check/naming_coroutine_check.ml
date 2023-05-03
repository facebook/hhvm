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

    method! at_expr env (_, p, e) =
      match e with
      | Await _ when is_sync env ->
        let func_pos =
          match env.function_name with
          | None -> None
          | Some sid -> Some (fst sid)
        in
        Errors.add_error
          Nast_check_error.(
            to_user_error @@ Await_in_sync_function { pos = p; func_pos })
      | _ -> ()

    method! at_stmt env =
      function
      | (_, Using { us_has_await; us_exprs; _ })
        when us_has_await && is_sync env ->
        Errors.add_error
          Nast_check_error.(
            to_user_error
            @@ Await_in_sync_function { pos = fst us_exprs; func_pos = None })
      | (_, Foreach (_, (Await_as_v (p, _) | Await_as_kv (p, _, _)), _))
      | (p, Awaitall _)
        when is_sync env ->
        Errors.add_error
          Nast_check_error.(
            to_user_error @@ Await_in_sync_function { pos = p; func_pos = None })
      | (p, Return (Some _)) when is_generator env ->
        Errors.add_error Nast_check_error.(to_user_error @@ Return_in_gen p)
      | _ -> ()
  end
