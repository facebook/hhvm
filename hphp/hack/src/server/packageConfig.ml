(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let pkgs_config_path_relative_to_repo_root = "PACKAGES.toml"

let repo_config_path =
  Relative_path.from_root ~suffix:pkgs_config_path_relative_to_repo_root

let log_debug (msg : ('a, unit, string, string, string, unit) format6) : 'a =
  Hh_logger.debug ~category:"packages" ?exn:None msg

let load_and_parse env : ServerEnv.env =
  let pkgs_config_abs_path = Relative_path.to_absolute repo_config_path in
  if not @@ Sys.file_exists pkgs_config_abs_path then
    env
  else
    let errors = Package.initialize_packages_info pkgs_config_abs_path in
    log_debug "Parsed %s" pkgs_config_abs_path;
    ServerEnv.
      {
        env with
        get_package_for_module = Some Package.get_package_for_module;
        errorl = Errors.merge env.errorl errors;
      }
