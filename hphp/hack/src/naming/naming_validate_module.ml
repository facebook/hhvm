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

let on_module_def :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.module_def * Err.t list ->
      ( Naming_phase_env.t * ('a, 'b) Aast_defs.module_def * Err.t list,
        'c )
      result =
 fun (env, (Aast.{ md_span; _ } as md), err_acc) ->
  let err =
    if Env.allow_module_def env then
      err_acc
    else
      (Err.naming
      @@ Naming_error.Module_declaration_outside_allowed_files md_span)
      :: err_acc
  in
  Ok (env, md, err)

let pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_module_def = Some on_module_def })
