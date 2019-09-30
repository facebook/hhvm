(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast

let error_if_is_this (pos, name) =
  if String.lowercase name = "this" then Errors.this_reserved pos

let error_if_does_not_start_with_T (pos, name) =
  if name.[0] <> 'T' then Errors.start_with_T pos

let check_constraint { Aast.tp_name = tp; _ } =
  error_if_is_this tp;
  error_if_does_not_start_with_T tp;
  ()

let error_if_contains_duplicates tparams =
  let _ =
    List.fold_left
      tparams
      ~init:SMap.empty
      ~f:(fun seen_tparams { Aast.tp_name = (pos, name); _ } ->
        match SMap.get name seen_tparams with
        | None -> SMap.add name pos seen_tparams
        | Some seen_pos ->
          Errors.shadowed_type_param pos seen_pos name;
          seen_tparams)
  in
  ()

let check_constraints tparams =
  error_if_contains_duplicates tparams;
  List.iter tparams ~f:check_constraint;
  ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ _ fun_ = check_constraints fun_.f_tparams

    method! at_class_ _ class_ =
      check_constraints class_.c_tparams.c_tparam_list

    method! at_method_ _ method_ = check_constraints method_.m_tparams

    method! at_typedef _ typedef = check_constraints typedef.t_tparams

    method! at_method_redeclaration _ method_redeclaration =
      check_constraints method_redeclaration.mt_tparams
  end
