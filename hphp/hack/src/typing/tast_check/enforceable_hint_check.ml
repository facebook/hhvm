(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Aast

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, _, e) =
      let validate hint op =
        let Equal = Tast_env.eq_typing_env in
        Typing_enforceable_hint.validate_hint env hint (fun pos reasons ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary
                @@ Primary.Invalid_is_as_expression_hint { op; pos; reasons }))
      in
      match e with
      | Is (_, hint) -> validate hint `is
      | As { hint; _ } -> validate hint `as_
      | _ -> ()
  end
