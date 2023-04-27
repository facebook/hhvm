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

let attribute_exists x1 attrs =
  List.exists attrs ~f:(fun { ua_name; _ } -> String.equal x1 (snd ua_name))

let static_memoized_check m =
  if attribute_exists SN.UserAttributes.uaMemoize m.m_user_attributes then
    Errors.add_nast_check_error
    @@ Nast_check_error.Static_memoized_function (fst m.m_name)

let unnecessary_memoize_lsb c m =
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
    Errors.add_error
      Naming_error.(
        to_user_error
        @@ Unnecessary_attribute
             { pos; attr; class_pos; class_name; suggestion })

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let disallow_static_memoized =
        TypecheckerOptions.experimental_feature_enabled
          (Tast_env.get_tcopt env)
          TypecheckerOptions.experimental_disallow_static_memoized
      in
      let (constructor, static_methods, _) = split_methods c.c_methods in
      if disallow_static_memoized && not c.c_final then (
        List.iter static_methods ~f:static_memoized_check;
        Option.iter constructor ~f:static_memoized_check
      );
      if c.c_final then List.iter static_methods ~f:(unnecessary_memoize_lsb c)
  end
