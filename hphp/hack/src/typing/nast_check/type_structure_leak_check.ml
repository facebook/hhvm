(*
 * Copyright (c) 2022, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let visitor =
  object (_this)
    inherit [_] Nast_visitor.iter_with_state as super

    method! on_expr (env, ancestor) ((_, _p, expr) as e) =
      let ancestor =
        match expr with
        | Is (_, _h) -> Some "an is-expression"
        | As { expr = _; hint = _; is_nullable = _; enforce_deep = _ } ->
          Some "an as-expression"
        | _ -> ancestor
      in
      super#on_expr (env, ancestor) e

    method! on_hint (env, ancestor) ((pos, h_) as h) =
      match h_ with
      | Hrefinement _ ->
        (match ancestor with
        | Some kind ->
          Errors.add_error
            Nast_check_error.(
              to_user_error @@ Refinement_in_typestruct { pos; kind })
        | None -> ())
      | _ -> super#on_hint (env, ancestor) h
  end

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr env = visitor#on_expr (env, None)

    method! at_class_ env class_ =
      List.iter class_.c_typeconsts ~f:(fun tc ->
          match tc.c_tconst_kind with
          | TCAbstract { c_atc_default = Some hint; _ }
          | TCConcrete { c_tc_type = hint; _ } ->
            let ancestor =
              Some
                (if tc.c_tconst_is_ctx then
                  "a context constant"
                else
                  "a type constant")
            in
            visitor#on_hint (env, ancestor) hint
          | _ -> ())

    method! at_typedef env td =
      visitor#on_hint (env, Some "a type alias") td.t_kind
  end
