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

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | ( _,
          pos,
          A.(
            ( Lfun
                ( {
                    f_fun_kind = Ast_defs.FSync;
                    f_body = { fb_ast = [(_, Return (Some (ty, _, e)))] };
                    _;
                  },
                  _ )
            | Efun
                {
                  ef_fun =
                    {
                      f_fun_kind = Ast_defs.FSync;
                      f_body = { fb_ast = [(_, Return (Some (ty, _, e)))] };
                      _;
                    };
                  _;
                } )) )
        when is_awaitable env ty -> begin
        match e with
        | A.Await _ -> ()
        | _ -> Lints_errors.async_lambda pos
      end
      | _ -> ()
  end
