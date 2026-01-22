(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let error_if_no_typehint
    { cst_mode; cst_type; cst_name; cst_value; _ } custom_err_config =
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
    Diagnostics.add_diagnostic
      (Naming_error_utils.to_user_diagnostic
         (Naming_error.Const_without_typehint { pos; const_name; ty_name })
         custom_err_config)

let error_if_pseudo_constant gconst custom_err_config =
  if Option.is_some gconst.cst_namespace.Namespace_env.ns_name then
    let (pos, name) = gconst.cst_name in
    let name = Utils.strip_all_ns name in
    if Naming_special_names.PseudoConsts.is_pseudo_const (Utils.add_ns name)
    then
      Diagnostics.add_diagnostic
        (Naming_error_utils.to_user_diagnostic
           (Naming_error.Name_is_reserved { pos; name })
           custom_err_config)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_gconst env gconst =
      let custom_err_config = Nast_check_env.get_custom_error_config env in
      error_if_no_typehint gconst custom_err_config;
      error_if_pseudo_constant gconst custom_err_config;
      ()
  end
