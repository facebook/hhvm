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

let on_class_
    on_error
    (Aast.{ c_span; c_kind; c_internal; c_user_attributes; _ } as c)
    ~ctx =
  let err_opt =
    if
      Ast_defs.is_c_trait c_kind
      && c_internal
      && Naming_attributes.mem
           SN.UserAttributes.uaModuleLevelTrait
           c_user_attributes
    then
      Some (Err.naming @@ Naming_error.Internal_module_level_trait c_span)
    else
      None
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok c)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_module_def =
          Some (fun elem ~ctx -> on_module_def on_error elem ~ctx);
        on_ty_class_ = Some (fun elem ~ctx -> on_class_ on_error elem ~ctx);
      }
