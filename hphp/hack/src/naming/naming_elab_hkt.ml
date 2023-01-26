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

let on_hint on_error (env, hint) =
  match hint with
  | (pos, Aast.Habstr (name, _ :: _)) when not @@ Env.hkt_enabled env ->
    on_error
      (Err.naming
      @@ Naming_error.Tparam_applied_to_type { pos; tparam_name = name });
    Ok (env, (pos, Aast.Habstr (name, [])))
  | _ -> Ok (env, hint)

let on_tparam on_error =
  let handler
        : 'a 'b.
          Naming_phase_env.t * ('a, 'b) Aast_defs.tparam ->
          (Naming_phase_env.t * ('a, 'b) Aast_defs.tparam, _) result =
   fun (env, (Aast.{ tp_parameters; tp_name = (pos, tparam_name); _ } as tparam))
       ->
    match tp_parameters with
    | _ :: _ when not @@ Env.hkt_enabled env ->
      on_error
        (Err.naming @@ Naming_error.Tparam_with_tparam { pos; tparam_name });
      Ok (env, Aast.{ tparam with tp_parameters = [] })
    | _ -> Ok (env, tparam)
  in
  handler

let pass on_error =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_hint = Some (on_hint on_error);
          on_tparam = Some (on_tparam on_error);
        })
