(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let pkgs_config_path_relative_to_repo_root = "__PACKAGES__.php"

let repo_config_path =
  Relative_path.from_root ~suffix:pkgs_config_path_relative_to_repo_root

let log_debug (msg : ('a, unit, string, string, string, unit) format6) : 'a =
  Hh_logger.debug ~category:"packages" ?exn:None msg

let load_and_parse (env : ServerEnv.env) :
    string -> Packages.package_info option =
  let ctx = Provider_utils.ctx_from_server_env env in
  let pkgs_config_abs_path = Relative_path.to_absolute repo_config_path in
  if Sys.file_exists pkgs_config_abs_path then (
    let (errors, ast) = Ast_provider.get_ast_with_error ctx repo_config_path in
    (* TODO(milliechen): make hh report errors *)
    if not (Errors.is_empty errors) then
      log_debug
        "Encountered errors while parsing %s:\n%s"
        pkgs_config_abs_path
        (Errors.show errors);

    log_debug "Parsed %s" pkgs_config_abs_path;
    let glob_to_package =
      List.fold ast ~init:SMap.empty ~f:(fun acc def ->
          let open Aast in
          match def with
          | Package { pkg_uses = Some mds; pkg_name; pkg_includes; _ } ->
            let pkg = Packages.{ pkg_name; pkg_includes; pkg_uses = mds } in
            List.fold mds ~init:acc ~f:(fun acc md ->
                SMap.add (module_name_kind_to_string md) pkg acc)
          | _ -> acc)
    in
    Packages.initialize glob_to_package
  );
  Packages.get_package_for_module
