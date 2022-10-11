(*
 * Copyright (c) 2022, Facebook, Inc.
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

let visitor =
  object (_this)
    inherit [_] Nast_visitor.iter_with_state as super

    method! on_expr (env, ancestor) ((_, _p, expr) as e) =
      let ancestor =
        match expr with
        | Is (_, _h) -> Some "an is-expression"
        | As (_, _h, _) -> Some "an as-expression"
        | _ -> ancestor
      in
      super#on_expr (env, ancestor) e

    method! on_hint (env, ancestor) ((pos, h_) as h) =
      match h_ with
      | Hrefinement _ ->
        (match ancestor with
        | Some kind ->
          Errors.add_nast_check_error
          @@ Nast_check_error.Refinement_in_typestruct { pos; kind }
        | None -> ())
      | _ -> super#on_hint (env, ancestor) h
  end

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr env = visitor#on_expr (env, None)
  end
