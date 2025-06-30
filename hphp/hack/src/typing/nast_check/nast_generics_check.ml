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

let error_if_is_this (pos, name) =
  if String.equal (String.lowercase name) "this" then
    Errors.add_error Naming_error.(to_user_error @@ This_reserved pos)

let error_if_invalid_tparam_name (pos, name) =
  if String.is_empty name || not (Char.equal name.[0] 'T') then
    Errors.add_error Naming_error.(to_user_error @@ Start_with_T pos)

(* See not on Naming.type_param about scoping of type parameters *)
let check_tparams (seen : tparam_info) tparams =
  let check_bind (seen : tparam_info) tparam =
    error_if_is_this tparam.tp_name;
    error_if_invalid_tparam_name tparam.tp_name;
    let (pos, name) = tparam.tp_name in
    if String.equal name Naming_special_names.Typehints.wildcard then (
      Errors.add_error
        Naming_error.(to_user_error @@ Wildcard_tparam_disallowed pos);
      seen
    ) else begin
      (match SMap.find_opt name seen with
      | Some prev_pos ->
        Errors.add_error
          Naming_error.(
            to_user_error
            @@ Shadowed_tparam { pos; prev_pos; tparam_name = name })
      | None -> ());
      SMap.add name pos seen
    end
  in

  List.fold_left tparams ~f:check_bind ~init:seen

let check_class class_ =
  let seen_class_tparams = check_tparams SMap.empty class_.c_tparams in

  (* Note that the class tparams are still marked as in scope *)
  let check_method method_tparams =
    check_tparams seen_class_tparams method_tparams |> ignore
  in

  List.iter class_.c_methods ~f:(fun m -> check_method m.m_tparams)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_def _ fd = check_tparams SMap.empty fd.fd_tparams |> ignore

    method! at_class_ _ = check_class

    method! at_typedef _ typedef =
      check_tparams SMap.empty typedef.t_tparams |> ignore
  end
