(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Env = struct
  let like_type_hints_enabled
      Naming_phase_env.{ like_type_hints_enabled; is_hhi; _ } =
    is_hhi || like_type_hints_enabled
end

let on_hint on_error hint ~ctx =
  let (err_opt, ctx) =
    match hint with
    | (pos, Aast.Hlike _) when not (Env.like_type_hints_enabled ctx) ->
      (Some (Naming_phase_error.like_type pos), ctx)
    | _ -> (None, ctx)
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok hint)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.{ id with on_ty_hint = Some (on_hint on_error) }
