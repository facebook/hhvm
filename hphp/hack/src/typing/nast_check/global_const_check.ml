(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let error_if_no_typehint gconst =
  if Partial_provider.should_check_error gconst.cst_mode 2001 then
    match gconst.cst_type with
    | None -> Errors.const_without_typehint gconst.cst_name
    | _ -> ()

let error_if_pseudo_constant gconst =
  if Option.is_some gconst.cst_namespace.Namespace_env.ns_name then
    let (pos, name) = gconst.cst_name in
    let name = Utils.strip_all_ns name in
    if Naming_special_names.PseudoConsts.is_pseudo_const (Utils.add_ns name)
    then
      Errors.name_is_reserved name pos

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_gconst _ gconst =
      error_if_no_typehint gconst;
      error_if_pseudo_constant gconst;
      ()
  end
