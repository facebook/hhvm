(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Typing_defs
module Env = Tast_env
module TCO = TypecheckerOptions

let should_enforce env =
  TCO.disallow_invalid_arraykey_constraint (Env.get_tcopt env)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_hint env (_, hint_) =
      match hint_ with
      | Hdarray (((p, _) as hkey), _) when should_enforce env ->
        let (env, ty) =
          hkey
          |> Env.hint_to_ty env
          |> Env.localize_with_self env ~ignore_errors:true
        in
        let (env, bound) =
          Typing_make_type.arraykey Reason.Rnone
          |> Typing_make_type.like Reason.Rnone
          |> Env.localize_with_self env ~ignore_errors:true
        in
        if not (Env.can_subtype env ty bound) then
          Errors.invalid_arraykey_constraint p (Env.print_error_ty env ty)
      | _ -> ()
  end
