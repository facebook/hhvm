(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = struct
  let soft_as_like Naming_phase_env.{ soft_as_like; _ } = soft_as_like
end

let on_hint hint ~ctx =
  match hint with
  | (pos, Aast.Hsoft hint) when Env.soft_as_like ctx ->
    (ctx, Ok (pos, Aast.Hlike hint))
  | (pos, Aast.Hsoft (_, hint_)) -> (ctx, Ok (pos, hint_))
  | _ -> (ctx, Ok hint)

let pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up Aast.Pass.{ id with on_ty_hint = Some on_hint }
