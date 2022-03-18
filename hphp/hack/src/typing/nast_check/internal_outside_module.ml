(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module SN = Naming_special_names

let has_internal_attribute =
  let f { ua_name; _ } =
    String.equal SN.UserAttributes.uaInternal (snd ua_name)
  in
  List.exists ~f

let check ~attributes ~module_ ~pos =
  if has_internal_attribute attributes && Option.is_none module_ then
    Errors.add_nast_check_error @@ Nast_check_error.Internal_outside_module pos

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      check
        ~attributes:c.c_user_attributes
        ~module_:env.Nast_check_env.module_
        ~pos:c.c_span

    method! at_method_ env m =
      check
        ~attributes:m.m_user_attributes
        ~module_:env.Nast_check_env.module_
        ~pos:m.m_span

    method! at_fun_ env f =
      check
        ~attributes:f.f_user_attributes
        ~module_:env.Nast_check_env.module_
        ~pos:f.f_span

    method! at_typedef env t =
      check
        ~attributes:t.t_user_attributes
        ~module_:env.Nast_check_env.module_
        ~pos:t.t_span
  end
