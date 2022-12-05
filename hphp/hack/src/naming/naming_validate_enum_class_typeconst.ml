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
  let allow_typeconst_in_enum_class
      Naming_phase_env.{ allow_typeconst_in_enum_class; _ } =
    allow_typeconst_in_enum_class
end

let on_class_ (env, (Aast.{ c_kind; c_typeconsts; c_span; _ } as c), err_acc) =
  let err =
    if
      (not @@ Env.allow_typeconst_in_enum_class env)
      && (not (List.is_empty c_typeconsts))
      && Ast_defs.is_c_enum_class c_kind
    then
      Err.Free_monoid.plus err_acc
      @@ Err.naming
      @@ Naming_error.Type_constant_in_enum_class_outside_allowed_locations
           c_span
    else
      err_acc
  in
  Naming_phase_pass.Cont.next (env, c, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_class_ = Some on_class_ })
