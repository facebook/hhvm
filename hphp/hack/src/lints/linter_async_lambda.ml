(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This lint advises users to use
      `async params ==> await ...`
    instead of
      `params ==> ...`
    when `...` is an `Awaitable`.

    The reason is the former produces better stack traces and gets optimised
    more aggressively.
  *)

module T = Typing_defs
module A = Aast_defs
module Reason = Typing_reason
module MakeType = Typing_make_type
module SN = Naming_special_names

let is_awaitable env ty =
  let (_env, ty) = Tast_env.expand_type env ty in
  match T.get_node ty with
  | T.Tclass ((_, id), _, _) -> String.equal id SN.Classes.cAwaitable
  | _ -> false

let strip_dynamic env ty =
  let tenv = Tast_env.tast_env_as_typing_env env in
  Typing_utils.strip_dynamic tenv ty

let is_awaitable_awaitable env ty =
  let ty = strip_dynamic env ty in
  let (env, ty) = Tast_env.expand_type env ty in
  match T.get_node ty with
  | T.Tclass ((_, id), _, [ty]) when String.equal id SN.Classes.cAwaitable ->
    let ty = strip_dynamic env ty in
    let (_env, ty) = Tast_env.expand_type env ty in
    begin
      match T.get_node ty with
      | T.Tclass ((_, id), _, _) -> String.equal id SN.Classes.cAwaitable
      | _ -> false
    end
  | _ -> false

let is_fsync = function
  | Ast_defs.FSync -> true
  | _ -> false

let is_await = function
  | A.Await _ -> true
  | _ -> false

let simple_body = function
  | [(_, A.Return (Some (ty, _, e)))] -> Some (ty, e)
  | _ -> None

let async_lambda_cond env ty e f_fun_kind =
  is_awaitable env ty && (not (is_await e)) && is_fsync f_fun_kind

let awaitable_awaitable_cond env = function
  | (ty, None) -> is_awaitable_awaitable env ty
  | _ -> false

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | ( _,
          pos,
          A.(
            ( Lfun ({ f_fun_kind; f_ret; f_body = { fb_ast }; _ }, _)
            | Efun { ef_fun = { f_fun_kind; f_ret; f_body = { fb_ast }; _ }; _ }
              )) ) ->
        (match simple_body fb_ast with
        | Some (ty, e) ->
          if async_lambda_cond env ty e f_fun_kind then
            Lints_errors.async_lambda pos
          else if awaitable_awaitable_cond env f_ret then
            Lints_errors.awaitable_awaitable pos
        | None ->
          if awaitable_awaitable_cond env f_ret then
            Lints_errors.awaitable_awaitable pos)
      | _ -> ()
  end
