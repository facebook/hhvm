(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let variadic_named_parameters Naming_phase_env.{ variadic_named_parameters; _ }
    =
  variadic_named_parameters

let check_params params =
  List.filter_map params ~f:(fun param ->
      match param with
      | Aast.
          {
            param_info = Param_variadic;
            param_named = Some Ast_defs.Param_named;
            param_pos;
            _;
          } ->
        Some
          (Err.naming
          @@ Naming_error.Variadic_named_parameter_disallowed param_pos)
      | _ -> None)

let on_method_ on_error m ~ctx =
  if not (variadic_named_parameters ctx) then
    List.iter ~f:on_error @@ check_params m.Aast.m_params;
  (ctx, Ok m)

let on_fun_ on_error f ~ctx =
  if not (variadic_named_parameters ctx) then
    List.iter ~f:on_error @@ check_params f.Aast.f_params;
  (ctx, Ok f)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_method_ = Some (fun elem ~ctx -> on_method_ on_error elem ~ctx);
        on_ty_fun_ = Some (fun elem ~ctx -> on_fun_ on_error elem ~ctx);
      }
