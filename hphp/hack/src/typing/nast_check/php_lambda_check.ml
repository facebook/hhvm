(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Nast_check_env

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr env (_, pos, expr) =
      if TypecheckerOptions.error_php_lambdas (get_tcopt env) then
        match expr with
        | Efun _ ->
          Errors.add_error
            Nast_check_error.(to_user_error @@ Php_lambda_disallowed pos)
        | _ -> ()
  end
