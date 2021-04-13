(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_reason
module SN = Naming_special_names
module MakeType = Typing_make_type
module Cls = Decl_provider.Class

let check_implements
    check_new_object
    attr_interface
    { Aast.ua_name = (attr_pos, attr_name); ua_params = params }
    env =
  let expr_kind =
    match SMap.find_opt attr_interface SN.AttributeKinds.plain_english_map with
    | Some ek -> ek
    | None -> "this expression"
    (* this case should never execute *)
  in
  let enable_systemlib_annotations =
    TypecheckerOptions.enable_systemlib_annotations (Typing_env.get_tcopt env)
  in
  if String_utils.string_starts_with attr_name "__" then
    (* Check against builtins *)
    let check_attr map =
      match SMap.find_opt attr_name map with
      | Some intfs ->
        let check_locations =
          TypecheckerOptions.check_attribute_locations
            (Typing_env.get_tcopt env)
        in
        if
          check_locations
          && (not @@ List.mem intfs attr_interface ~equal:String.equal)
        then
          Errors.wrong_expression_kind_builtin_attribute
            expr_kind
            attr_pos
            attr_name;
        true
      | None -> false
    in
    let () =
      if
        check_attr SN.UserAttributes.as_map
        || enable_systemlib_annotations
           && check_attr SN.UserAttributes.systemlib_map
      then
        ()
      else
        Errors.unbound_attribute_name attr_pos attr_name
    in
    env
  else
    match
      ( Typing_env.get_class env attr_name,
        Typing_env.get_class env attr_interface )
    with
    | (Some attr_class, Some intf_class) ->
      (* Found matching class *)
      let attr_cid = (Cls.pos attr_class, Cls.name attr_class) in
      (* successful exit condition: attribute class is subtype of correct interface
       * and its args satisfy the attribute class constructor *)
      let attr_locl_ty : Typing_defs.locl_ty =
        MakeType.class_type
          (Rwitness_from_decl (Cls.pos attr_class))
          (Cls.name attr_class)
          []
      in
      let interface_locl_ty : Typing_defs.locl_ty =
        MakeType.class_type
          (Rwitness_from_decl (Cls.pos intf_class))
          (Cls.name intf_class)
          []
      in
      if not (Typing_subtype.is_sub_type env attr_locl_ty interface_locl_ty)
      then (
        Errors.wrong_expression_kind_attribute
          expr_kind
          attr_pos
          attr_name
          (Cls.pos attr_class)
          (Cls.name attr_class)
          (Cls.name intf_class);
        env
      ) else
        let (env, _, _, _, _, _, _) =
          check_new_object
            ~expected:None
            ~check_parent:false
            ~check_not_abstract:false
            ~is_using_clause:false
            attr_pos
            env
            (Aast.CI (Positioned.unsafe_to_raw_positioned attr_cid))
            []
            params (* list of attr parameter literals *)
            None
          (* no variadic arguments *)
        in
        env
    | _ ->
      Errors.unbound_attribute_name attr_pos attr_name;
      env

let check_def env check_new_object kind f_attrs =
  List.fold_right ~f:(check_implements check_new_object kind) f_attrs ~init:env
