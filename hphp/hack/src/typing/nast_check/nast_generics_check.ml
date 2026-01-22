(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

(** Positions of type parameters that are in scope. *)
type tparam_info = pos SMap.t

let error_if_is_this (pos, name) custom_err_config =
  if String.equal (String.lowercase name) "this" then
    Diagnostics.add_diagnostic
      (Naming_error_utils.to_user_diagnostic
         (Naming_error.This_reserved pos)
         custom_err_config)

let error_if_invalid_tparam_name (pos, name) custom_err_config =
  if String.is_empty name || not (Char.equal name.[0] 'T') then
    Diagnostics.add_diagnostic
      (Naming_error_utils.to_user_diagnostic
         (Naming_error.Start_with_T pos)
         custom_err_config)

(* See not on Naming.type_param about scoping of type parameters *)
let check_tparams (seen : tparam_info) tparams custom_err_config =
  let check_bind (seen : tparam_info) tparam =
    error_if_is_this tparam.tp_name custom_err_config;
    error_if_invalid_tparam_name tparam.tp_name custom_err_config;
    let (pos, name) = tparam.tp_name in
    if String.equal name Naming_special_names.Typehints.wildcard then (
      Diagnostics.add_diagnostic
        (Naming_error_utils.to_user_diagnostic
           (Naming_error.Wildcard_tparam_disallowed pos)
           custom_err_config);
      seen
    ) else begin
      (match SMap.find_opt name seen with
      | Some prev_pos ->
        Diagnostics.add_diagnostic
          (Naming_error_utils.to_user_diagnostic
             (Naming_error.Shadowed_tparam { pos; prev_pos; tparam_name = name })
             custom_err_config)
      | None -> ());
      SMap.add name pos seen
    end
  in

  List.fold_left tparams ~f:check_bind ~init:seen

let check_class class_ custom_err_config =
  let seen_class_tparams =
    check_tparams SMap.empty class_.c_tparams custom_err_config
  in

  (* Note that the class tparams are still marked as in scope *)
  let check_method method_tparams =
    ignore (check_tparams seen_class_tparams method_tparams custom_err_config)
  in

  List.iter class_.c_methods ~f:(fun m -> check_method m.m_tparams)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_def env fd =
      let custom_err_config = Nast_check_env.get_custom_error_config env in
      ignore (check_tparams SMap.empty fd.fd_tparams custom_err_config)

    method! at_class_ env cls =
      let custom_err_config = Nast_check_env.get_custom_error_config env in
      check_class cls custom_err_config

    method! at_typedef env typedef =
      let custom_err_config = Nast_check_env.get_custom_error_config env in
      ignore (check_tparams SMap.empty typedef.t_tparams custom_err_config)
  end
