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

  let everything_sdt Naming_phase_env.{ everything_sdt; _ } = everything_sdt
end

let on_hint_ on_error (env, hint_) =
  let err_opt =
    if
      Env.is_hhi env
      || Env.is_systemlib env
      || Env.supportdynamic_type_hint_enabled env
      || Env.everything_sdt env
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
  Ok (env, hint_)

let pass on_error =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_hint_ = Some (on_hint_ on_error) })
