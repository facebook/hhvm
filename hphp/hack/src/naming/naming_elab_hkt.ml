(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env = struct
  let hkt_enabled Naming_phase_env.{ hkt_enabled; _ } = hkt_enabled
end

let on_hint (env, hint, err_acc) =
  match hint with
  | (pos, Aast.Habstr (name, _ :: _)) when not @@ Env.hkt_enabled env ->
    let err =
      (Err.naming
      @@ Naming_error.Tparam_applied_to_type { pos; tparam_name = name })
      :: err_acc
    in
    Ok (env, (pos, Aast.Habstr (name, [])), err)
  | _ -> Ok (env, hint, err_acc)

let on_tparam
    ( env,
      (Aast.{ tp_parameters; tp_name = (pos, tparam_name); _ } as tparam),
      err_acc ) =
  match tp_parameters with
  | _ :: _ when not @@ Env.hkt_enabled env ->
    let err =
      (Err.naming @@ Naming_error.Tparam_with_tparam { pos; tparam_name })
      :: err_acc
    in
    Ok (env, Aast.{ tparam with tp_parameters = [] }, err)
  | _ -> Ok (env, tparam, err_acc)

let pass =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        { identity with on_hint = Some on_hint; on_tparam = Some on_tparam })
