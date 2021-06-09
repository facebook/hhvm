(*
 * Copyright () 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Nast_check_env
module SN = Naming_special_names

let error_if_static_prop_is_const env cv =
  if
    cv.cv_is_static
    && (not (TypecheckerOptions.const_static_props (get_tcopt env)))
    && Naming_attributes.mem SN.UserAttributes.uaConst cv.cv_user_attributes
  then
    let pos = fst cv.cv_id in
    Errors.experimental_feature pos "Const properties cannot be static."

(* Non-static properties cannot have attribute __LSB *)
let error_if_nonstatic_prop_with_lsb cv =
  if not cv.cv_is_static then
    let lsb_pos =
      Naming_attributes.mem_pos SN.UserAttributes.uaLSB cv.cv_user_attributes
    in
    Option.iter lsb_pos ~f:Errors.nonstatic_property_with_lsb

let unnecessary_lsb c cv =
  let attr = SN.UserAttributes.uaLSB in
  match Naming_attributes.mem_pos attr cv.cv_user_attributes with
  | None -> ()
  | Some pos ->
    let (pos_class, name_class) = c.c_name in
    let name_class = Utils.strip_ns name_class in
    let reason = (pos_class, sprintf "the class `%s` is final" name_class) in
    let suggestion = None in
    Errors.unnecessary_attribute pos ~attr ~reason ~suggestion

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env cv =
      let check_vars cv =
        error_if_static_prop_is_const env cv;
        error_if_nonstatic_prop_with_lsb cv;
        ()
      in
      List.iter cv.c_vars ~f:check_vars;
      if cv.c_final then List.iter cv.c_vars ~f:(unnecessary_lsb cv)
  end
