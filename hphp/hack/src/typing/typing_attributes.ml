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
open Typing_env_types
module SN = Naming_special_names
module MakeType = Typing_make_type
module Cls = Decl_provider.Class

type attribute_interface_name = string

type new_object_checker =
  Pos.t -> env -> Typing_defs.pos_string -> Nast.expr list -> env

let check_implements
    (check_new_object : new_object_checker)
    (attr_interface : attribute_interface_name)
    ({ Aast.ua_name = (attr_pos, attr_name); ua_params = params } :
      Nast.user_attribute)
    (env : env) : env =
  let expr_kind =
    match SMap.find_opt attr_interface SN.AttributeKinds.plain_english_map with
    | Some ek -> ek
    | None -> "this expression"
    (* this case should never execute *)
  in
  let is_systemlib =
    TypecheckerOptions.is_systemlib (Typing_env.get_tcopt env)
  in
  if String.is_prefix attr_name ~prefix:"__" then
    (* Check against builtins *)
    let check_attr map =
      match SMap.find_opt attr_name map with
      | Some attr_info ->
        let check_locations =
          TypecheckerOptions.check_attribute_locations
            (Typing_env.get_tcopt env)
        in
        if
          check_locations
          && not
             @@ List.mem
                  attr_info.SN.UserAttributes.contexts
                  attr_interface
                  ~equal:String.equal
        then
          Errors.add_error
            Nast_check_error.(
              to_user_error
              @@ Wrong_expression_kind_builtin_attribute
                   { expr_kind; pos = attr_pos; attr_name });
        true
      | None -> false
    in
    let () =
      if
        check_attr SN.UserAttributes.as_map
        || (is_systemlib && check_attr SN.UserAttributes.systemlib_map)
      then
        ()
      else
        let all_valid_user_attributes =
          let bindings = SMap.bindings SN.UserAttributes.as_map in
          let filtered_bindings =
            List.filter
              ~f:(fun (_, attr_info) ->
                List.mem
                  attr_info.SN.UserAttributes.contexts
                  attr_interface
                  ~equal:String.equal)
              bindings
          in
          List.map ~f:fst filtered_bindings
        in

        let closest_attr_name =
          Typing_env.most_similar
            attr_name
            all_valid_user_attributes
            (fun name -> name)
        in

        Errors.add_error
          Naming_error.(
            to_user_error
            @@ Unbound_attribute_name
                 { pos = attr_pos; attr_name; closest_attr_name })
    in

    env
  else
    match
      ( Typing_env.get_class env attr_name,
        Typing_env.get_class env attr_interface )
    with
    | (Decl_entry.Found attr_class, Decl_entry.Found intf_class) ->
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
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Wrong_expression_kind_attribute
                 {
                   expr_kind;
                   pos = attr_pos;
                   attr_name;
                   attr_class_pos = Cls.pos attr_class;
                   attr_class_name = Cls.name attr_class;
                   intf_name = Cls.name intf_class;
                 });
        env
      ) else
        check_new_object attr_pos env attr_cid params
    | (Decl_entry.NotYetAvailable, _)
    | (_, Decl_entry.NotYetAvailable) ->
      (* A retry will happen. Don't emit error to avoid error-based backtracking *)
      env
    | _ ->
      Errors.add_error
        Naming_error.(
          to_user_error
          @@ Unbound_attribute_name
               { pos = attr_pos; attr_name; closest_attr_name = None });
      env

let check_def env check_new_object (kind : attribute_interface_name) attributes
    =
  List.fold_right
    ~f:(check_implements check_new_object kind)
    attributes
    ~init:env

(* TODO(coeffects) change to mixed after changing those constructors to pure *)
let check_def env check_new_object (kind : attribute_interface_name) attributes
    =
  let defaults = MakeType.default_capability Pos_or_decl.none in
  let (env, _) =
    Typing_lenv.stash_and_do env (Typing_env.all_continuations env) (fun env ->
        let env =
          fst @@ Typing_coeffects.register_capabilities env defaults defaults
        in
        (check_def env check_new_object kind attributes, ()))
  in
  env
