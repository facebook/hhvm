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

let on_module_def (env, (Aast.{ md_span; _ } as md), err) =
  let err =
    if Env.allow_module_def env then
      err
    else
      Err.Free_monoid.plus err
      @@ Err.naming
      @@ Naming_error.Module_declaration_outside_allowed_files md_span
  in
  Naming_phase_pass.Cont.next (env, md, err)

let pass =
  Naming_phase_pass.(
    top_down { identity with on_module_def = Some on_module_def })
