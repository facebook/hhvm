(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let error_if_no_typehint { cst_mode; cst_type; cst_name; cst_value; _ } =
  if (not (FileInfo.is_hhi cst_mode)) && Option.is_none cst_type then
    let (_, _, expr) = cst_value in
    let (pos, const_name) = cst_name
    and ty_name =
      match expr with
      | String _ -> "string"
      | Int _ -> "int"
      | Float _ -> "float"
      | _ -> "mixed"
    in
    Errors.add_error
      Naming_error.(
        to_user_error @@ Const_without_typehint { pos; const_name; ty_name })

let error_if_pseudo_constant gconst =
  if Option.is_some gconst.cst_namespace.Namespace_env.ns_name then
    let (pos, name) = gconst.cst_name in
    let name = Utils.strip_all_ns name in
    if Naming_special_names.PseudoConsts.is_pseudo_const (Utils.add_ns name)
    then
      Errors.add_error
        Naming_error.(to_user_error @@ Name_is_reserved { pos; name })

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_gconst _ gconst =
      error_if_no_typehint gconst;
      error_if_pseudo_constant gconst;
      ()
  end
