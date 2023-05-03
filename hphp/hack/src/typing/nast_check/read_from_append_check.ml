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

    method! at_expr env (_, _, e) =
      match e with
      | Array_get ((_, p, _), None) when not env.array_append_allowed ->
        Errors.add_error
          Nast_check_error.(to_user_error @@ Reading_from_append p)
      | _ -> ()
  end
