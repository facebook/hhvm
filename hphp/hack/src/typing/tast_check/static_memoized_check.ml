(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module SN = Naming_special_names

let unnecessary_memoize_lsb c m custom_err_config =
  let attr = SN.UserAttributes.uaMemoizeLSB in
  match Naming_attributes.mem_pos attr m.m_user_attributes with
  | None -> ()
  | Some pos ->
    let (class_pos, class_name) = c.c_name in
    let suggestion =
      Some
        (sprintf
           "Try using the attribute `%s` instead"
           SN.UserAttributes.uaMemoize)
    in
    Diagnostics.add_diagnostic
      (Naming_error_utils.to_user_diagnostic
         (Naming_error.Unnecessary_attribute
            { pos; attr; class_pos; class_name; suggestion })
         custom_err_config)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let custom_err_config =
        let tcopt = Tast_env.get_tcopt env in
        TypecheckerOptions.custom_error_config tcopt
      in
      let (_, static_methods, _) = split_methods c.c_methods in
      if c.c_final then
        List.iter static_methods ~f:(fun m ->
            unnecessary_memoize_lsb c m custom_err_config)
  end
