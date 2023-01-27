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

let on_hint on_error hint ~ctx =
  match hint with
  | (pos, Aast.Habstr (name, _ :: _)) when not @@ Env.hkt_enabled ctx ->
    on_error
      (Err.naming
      @@ Naming_error.Tparam_applied_to_type { pos; tparam_name = name });
    (ctx, Ok (pos, Aast.Habstr (name, [])))
  | _ -> (ctx, Ok hint)

let on_tparam
    on_error
    (Aast.{ tp_parameters; tp_name = (pos, tparam_name); _ } as tparam)
    ~ctx =
  match tp_parameters with
  | _ :: _ when not @@ Env.hkt_enabled ctx ->
    on_error (Err.naming @@ Naming_error.Tparam_with_tparam { pos; tparam_name });
    (ctx, Ok Aast.{ tparam with tp_parameters = [] })
  | _ -> (ctx, Ok tparam)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_hint = Some (on_hint on_error);
        on_ty_tparam = Some (fun elem ~ctx -> on_tparam on_error elem ~ctx);
      }
