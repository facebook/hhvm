(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_reason

module SN = Naming_special_names
module MakeType = Typing_make_type
module Cls = Typing_classes_heap

let check_implements check_new_object attr_interface
  { Nast.ua_name = (attr_pos, attr_name)
  ; Nast.ua_params = params } env =
  if String_utils.string_starts_with attr_name "__"
  then begin
    (* Check against builtins *)
    if not (SSet.mem attr_name SN.UserAttributes.as_set)
    then Errors.unbound_attribute_name attr_pos attr_name; env end
  else
    match Typing_env.get_class env attr_name, Typing_env.get_class env attr_interface with
    | Some attr_class, Some intf_class ->
      (* Found matching class *)
      let attr_cid = (Cls.pos attr_class, Cls.name attr_class) in
      (* successful exit condition: attribute class is subtype of correct interface
       * and its args satisfy the attribute class constructor *)
     let attr_locl_ty: (Typing_defs.locl Typing_defs.ty) =
       MakeType.class_type (Rwitness (Cls.pos attr_class)) (Cls.name attr_class) [] in
     let interface_locl_ty: (Typing_defs.locl Typing_defs.ty) =
       MakeType.class_type (Rwitness (Cls.pos intf_class)) (Cls.name intf_class) [] in
      if not (Typing_subtype.is_sub_type env attr_locl_ty interface_locl_ty)
      then begin
        let expr_kind =
          match SMap.get attr_interface SN.AttributeKinds.plain_english_map  with
          | Some ek -> ek
          | None -> "this expression" (* this case should never execute *) in
        Errors.wrong_expression_kind_attribute expr_kind
          attr_pos attr_name (Cls.pos attr_class) (Cls.name attr_class) (Cls.name intf_class);
        env end
      else
        let env, _, _, _, _, _ = check_new_object
          ~expected:None
          ~check_parent:false
          ~check_not_abstract:false
          ~is_using_clause:false attr_pos env
          (Nast.CI attr_cid)
          []
          params (* list of attr parameter literals *)
          [] (* no variadic arguments *) in
        env
    | _ ->
      Errors.unbound_attribute_name attr_pos attr_name; env

let check_def env check_new_object kind f_attrs =
  List.fold_right ~f:(check_implements check_new_object kind) f_attrs ~init:env
