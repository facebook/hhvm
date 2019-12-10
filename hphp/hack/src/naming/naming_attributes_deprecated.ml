(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module SN = Naming_special_names

(* TODO: generalize the arity check / argument check here to handle attributes
 * in general, not just __Deprecated *)
let deprecated ~kind (_, name) attrs =
  let attr = Naming_attributes.find SN.UserAttributes.uaDeprecated attrs in
  let open Aast in
  match attr with
  | Some { ua_name = _; ua_params = [msg] }
  | Some { ua_name = _; ua_params = [msg; _] } ->
    begin
      match Nast_eval.static_string msg with
      | Ok msg ->
        let name = Utils.strip_ns name in
        let deprecated_prefix =
          Printf.sprintf "The %s %s is deprecated: " kind name
        in
        Some (deprecated_prefix ^ msg)
      | Error p ->
        Errors.attribute_param_type p "static string literal";
        None
    end
  | Some { ua_name = (pos, _); ua_params = [] } ->
    Errors.attribute_too_few_arguments pos SN.UserAttributes.uaDeprecated 1;
    None
  | Some { ua_name = (pos, _); ua_params = _ } ->
    Errors.attribute_too_many_arguments pos SN.UserAttributes.uaDeprecated 2;
    None
  | None -> None
