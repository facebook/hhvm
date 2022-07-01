(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Aast

let check_statement_in_function ((_, stmt_) : Tast.stmt) (fun_name : string) =
  match stmt_ with
  | Aast.Expr (_ty, _pos, expr_) ->
    (match expr_ with
    | Aast.Call ((_, _, Aast.Id (_, name)), _, _, _) ->
      String.equal name fun_name
    | _ -> false)
  | _ -> false

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def _env f =
      if Hh_prelude.List.is_empty f.fd_fun.f_params then
        match f.fd_fun.f_body.fb_ast with
        | head :: _ ->
          if check_statement_in_function head (snd f.fd_fun.f_name) then
            Lints_errors.unconditional_recursion f.fd_fun.f_span
          else
            ()
        | [] -> ()
      else
        ()
  end
