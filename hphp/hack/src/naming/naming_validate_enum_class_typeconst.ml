(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Env = struct
  let allow_typeconst_in_enum_class
      Naming_phase_env.{ allow_typeconst_in_enum_class; _ } =
    allow_typeconst_in_enum_class
end

let on_class_ on_error (Aast.{ c_kind; c_typeconsts; c_span; _ } as c) ~ctx =
  let err_opt =
    if
      (not @@ Env.allow_typeconst_in_enum_class ctx)
      && (not (List.is_empty c_typeconsts))
      && Ast_defs.is_c_enum_class c_kind
    then
      Some
        (Naming_phase_error.naming
        @@ Naming_error.Type_constant_in_enum_class_outside_allowed_locations
             c_span)
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
        on_ty_class_ = Some (fun elem ~ctx -> on_class_ on_error elem ~ctx);
      }
