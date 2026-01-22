(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
module SN = Naming_special_names

let lookup_package env p =
  let info =
    Provider_context.get_tcopt env.Nast_check_env.ctx
    |> TypecheckerOptions.package_info
  in
  PackageInfo.get_package info p

let require_package_strict_inclusion env attr =
  if
    Provider_context.get_tcopt env.Nast_check_env.ctx
    |> TypecheckerOptions.check_packages
  then
    match
      Naming_attributes.find2
        SN.UserAttributes.uaRequirePackage
        SN.UserAttributes.uaSoftRequirePackage
        attr
    with
    | Some
        {
          ua_params = [(_, _, String required_pkg_name)];
          ua_name = (name_pos, name);
        } ->
      let required_pkg = lookup_package env required_pkg_name in
      (match (required_pkg, env.Nast_check_env.package) with
      | (None, _) when String.equal name SN.UserAttributes.uaSoftRequirePackage
        ->
        (* Emit naming error for unbound package *)
        let custom_err_config =
          Provider_context.get_tcopt env.Nast_check_env.ctx
          |> TypecheckerOptions.custom_error_config
        in
        Diagnostics.add_diagnostic
          (Naming_error_utils.to_user_diagnostic
             (Naming_error.Unbound_name
                {
                  pos = name_pos;
                  name = required_pkg_name;
                  kind = Name_context.PackageNamespace;
                })
             custom_err_config)
      | (Some required_package, Some current_pkg_membership) ->
        let ( current_pkg,
              current_pkg_name,
              current_pkg_pos,
              current_package_assignment_kind ) =
          match current_pkg_membership with
          | Aast_defs.PackageConfigAssignment pkg_name ->
            let pkg = lookup_package env pkg_name in
            let pos =
              match pkg with
              | Some p -> Package.get_package_pos p
              | None -> Pos.none
            in
            (pkg, pkg_name, pos, "package config assignment")
          | Aast_defs.PackageOverride (pkg_pos, pkg_name) ->
            let pkg = lookup_package env pkg_name in
            (pkg, pkg_name, pkg_pos, "package override")
        in
        (match current_pkg with
        | Some current_package ->
          (match Package.relationship required_package current_package with
          | Package.Includes -> ()
          | Package.Equal
          | Package.Unrelated
          | Package.Soft_includes ->
            let required_pos = Package.get_package_pos required_package in
            Diagnostics.add_diagnostic
              Nast_check_error.(
                to_user_diagnostic
                @@ Require_package_strict_inclusion
                     {
                       required_pos = name_pos;
                       required = required_pkg_name;
                       def_pos = Pos_or_decl.of_raw_pos required_pos;
                       current = current_pkg_name;
                       current_pos = current_pkg_pos;
                       attribute_name = name;
                       soft_included = false;
                       current_package_assignment_kind;
                     }));
          (* Check the opposite direction *)
          (match Package.relationship current_package required_package with
          | Package.Soft_includes ->
            let required_pos = Package.get_package_pos current_package in
            Diagnostics.add_diagnostic
              Nast_check_error.(
                to_user_diagnostic
                @@ Require_package_strict_inclusion
                     {
                       required_pos = name_pos;
                       required = required_pkg_name;
                       def_pos = Pos_or_decl.of_raw_pos required_pos;
                       current = current_pkg_name;
                       current_pos = current_pkg_pos;
                       attribute_name = name;
                       soft_included = true;
                       current_package_assignment_kind;
                     })
          | _ -> ())
        | None -> ())
      | _ -> ())
    | _ -> ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_def env f =
      require_package_strict_inclusion env f.fd_fun.f_user_attributes

    method! at_method_ env m =
      require_package_strict_inclusion env m.m_user_attributes
  end
