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

let check ~is_internal ~module_ ~pos =
  if is_internal && Option.is_none module_ then
    Errors.add_nast_check_error @@ Nast_check_error.Internal_outside_module pos

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      check
        ~is_internal:c.c_internal
        ~module_:env.Nast_check_env.module_
        ~pos:c.c_span

    method! at_method_ env m =
      check
        ~is_internal:(Aast.equal_visibility m.m_visibility Aast.Internal)
        ~module_:env.Nast_check_env.module_
        ~pos:m.m_span

    method! at_fun_def env fd =
      check
        ~is_internal:fd.fd_internal
        ~module_:env.Nast_check_env.module_
        ~pos:fd.fd_fun.f_span

    method! at_typedef env t =
      check
        ~is_internal:t.t_internal
        ~module_:env.Nast_check_env.module_
        ~pos:t.t_span
  end
