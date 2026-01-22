(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_env_types
module SN = Naming_special_names
module MakeType = Typing_make_type
module Cls = Folded_class

type attribute_interface_name = string

type new_object_checker =
  Pos.t -> env -> Typing_defs.pos_string -> Nast.argument list -> env

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
          Diagnostics.add_diagnostic
            Nast_check_error.(
              to_user_diagnostic
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

        let custom_err_config =
          TypecheckerOptions.custom_error_config (Typing_env.get_tcopt env)
        in
        Diagnostics.add_diagnostic
          (Naming_error_utils.to_user_diagnostic
             (Naming_error.Unbound_attribute_name
                { pos = attr_pos; attr_name; closest_attr_name })
             custom_err_config)
    in

    env
  else
    match Typing_env.get_class env attr_name with
    | Decl_entry.Found attr_class ->
      (* Found matching class *)
      let attr_cid = (Cls.pos attr_class, Cls.name attr_class) in
      if not (Cls.has_ancestor attr_class attr_interface) then (
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
                   intf_name = attr_interface;
                 });
        env
      ) else
        check_new_object
          attr_pos
          env
          attr_cid
          (List.map ~f:(fun e -> Aast_defs.Anormal e) params)
    | Decl_entry.NotYetAvailable ->
      (* A retry will happen. Don't emit error to avoid error-based backtracking *)
      env
    | _ ->
      let custom_err_config =
        TypecheckerOptions.custom_error_config (Typing_env.get_tcopt env)
      in
      Diagnostics.add_diagnostic
        (Naming_error_utils.to_user_diagnostic
           (Naming_error.Unbound_attribute_name
              { pos = attr_pos; attr_name; closest_attr_name = None })
           custom_err_config);
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
