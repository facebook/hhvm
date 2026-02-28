(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  let is_hhi Naming_phase_env.{ is_hhi; _ } = is_hhi

  let is_systemlib Naming_phase_env.{ is_systemlib; _ } = is_systemlib

  let supportdynamic_type_hint_enabled
      Naming_phase_env.{ supportdynamic_type_hint_enabled; _ } =
    supportdynamic_type_hint_enabled
end

let on_hint_ on_error hint_ ~ctx =
  let err_opt =
    if
      Env.is_hhi ctx
      || Env.is_systemlib ctx
      || Env.supportdynamic_type_hint_enabled ctx
    then
      None
    else
      match hint_ with
      | Aast.Happly ((pos, ty_name), _)
        when String.(equal ty_name SN.Classes.cSupportDyn) ->
        Some (Err.supportdyn pos)
      | _ -> None
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok hint_)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.{ id with on_ty_hint_ = Some (on_hint_ on_error) }
