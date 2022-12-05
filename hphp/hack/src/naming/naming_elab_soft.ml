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

let on_hint (env, hint, err) =
  let hint =
    match hint with
    | (pos, Aast.Hsoft hint) when Env.soft_as_like env -> (pos, Aast.Hlike hint)
    | (pos, Aast.Hsoft (_, hint_)) -> (pos, hint_)
    | _ -> hint
  in
  Naming_phase_pass.Cont.next (env, hint, err)

let pass = Naming_phase_pass.(top_down { identity with on_hint = Some on_hint })
