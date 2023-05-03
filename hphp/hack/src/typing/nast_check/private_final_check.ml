(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Aast

(* Ban `private final` on classes, but not traits. *)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_method_ env m =
      match (env.Nast_check_env.classish_kind, m.m_visibility, m.m_final) with
      | (Some Ast_defs.Ctrait, _, _) -> ()
      | (_, Private, true) ->
        let (pos, _) = m.m_name in
        Errors.add_error
          Nast_check_error.(to_user_error @@ Private_and_final pos)
      | _ -> ()
  end
