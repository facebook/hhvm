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
  let allow_module_def Naming_phase_env.{ allow_module_def; _ } =
    allow_module_def
end

let on_module_def on_error (Aast.{ md_span; _ } as md) ~ctx =
  let err_opt =
    if Env.allow_module_def ctx then
      None
    else
      Some
        (Err.naming
        @@ Naming_error.Module_declaration_outside_allowed_files md_span)
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok md)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_module_def =
          Some (fun elem ~ctx -> on_module_def on_error elem ~ctx);
      }
