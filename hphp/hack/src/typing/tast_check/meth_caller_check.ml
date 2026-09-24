(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs

let check_inout_parameters env pos (_, ftype) =
  match
    List.find
      ~f:(fun ft_param ->
        match get_fp_mode ft_param with
        | FPnormal -> false
        | FPinout -> true)
      ftype.ft_params
  with
  | Some fparam ->
    let Equal = Tast_env.eq_typing_env in
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Invalid_meth_caller_inout_parameter
             { pos; decl_pos = fparam.fp_pos })
  | None -> ()

let check_named_parameters env pos (_, ftype) =
  match List.find ~f:get_fp_is_named ftype.ft_params with
  | Some fparam ->
    let Equal = Tast_env.eq_typing_env in
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Invalid_meth_caller_named_parameter
             { pos; decl_pos = fparam.fp_pos })
  | None -> ()

let check_readonly_return env pos (r, ftype) =
  let Equal = Tast_env.eq_typing_env in
  if Flags.get_ft_returns_readonly ftype then
    let rpos = Typing_reason.to_pos r in
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Invalid_meth_caller_readonly_return { pos; decl_pos = rpos })

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e =
      match e with
      | (ty, pos, Method_caller _) -> begin
        match Tast_env.get_underlying_function_type env ty with
        | None -> ()
        | Some ft ->
          let Equal = Tast_env.eq_typing_env in
          check_inout_parameters env pos ft;
          check_named_parameters env pos ft;
          check_readonly_return env pos ft
      end
      | _ -> ()
  end
