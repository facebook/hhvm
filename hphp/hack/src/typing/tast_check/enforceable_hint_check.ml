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

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e =
      let validate hint op =
        Typing_enforceable_hint.validate_hint
          env
          hint
          (Errors.invalid_is_as_expression_hint op)
      in
      match snd e with
      | Is (_, hint) -> validate hint "is"
      | As (_, hint, _) -> validate hint "as"
      | _ -> ()
  end
